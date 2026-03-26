// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include "Alimer/Core/Vector.h"
#include "Alimer/Core/Stopwatch.h"
#include "Alimer/Core/JobSystem.h"
#include <chrono>

#include <memory>
#include <algorithm>
#include <deque>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#if defined(_WIN32)
#include "Alimer/Platform/Win32/WindowsPlatform.h"
#include <intrin.h>
#elif ALIMER_PLATFORM_LINUX
#include <pthread.h>
#include <sys/resource.h>
#endif

namespace
{
#if defined(_WIN32)
    inline long AtomicAdd(volatile long* ptr, long val)
    {
        return _InterlockedExchangeAdd(ptr, val);
    }
    inline long AtomicOr(volatile long* ptr, long mask)
    {
        return _InterlockedOr(ptr, mask);
    }
#else
    inline long AtomicAdd(volatile long* ptr, long val)
    {
        return __atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST);
    }
    inline long AtomicOr(volatile long* ptr, long mask)
    {
        return __atomic_fetch_or(ptr, mask, __ATOMIC_SEQ_CST);
    }
#endif
    inline long AtomicLoad(const volatile long* ptr)
    {
        return AtomicOr((volatile long*)ptr, 0);
    }
}

namespace Alimer::JobSystem
{
    struct Job final
    {
        std::function<void(JobArgs)> task;
        Context* ctx;
        uint32_t groupID;
        uint32_t groupJobOffset;
        uint32_t groupJobEnd;
        uint32_t sharedMemorySize;

        inline void Execute()
        {
            JobArgs args;
            args.groupID = groupID;
            if (sharedMemorySize > 0)
            {
                args.sharedMemory = alloca(sharedMemorySize);
            }
            else
            {
                args.sharedMemory = nullptr;
            }

            for (uint32_t j = groupJobOffset; j < groupJobEnd; ++j)
            {
                args.jobIndex = j;
                args.groupIndex = j - groupJobOffset;
                args.isFirstJobInGroup = (j == groupJobOffset);
                args.isLastJobInGroup = (j == groupJobEnd - 1);
                task(args);
            }

            AtomicAdd(&ctx->counter, -1);
        }
    };

    struct JobQueue final
    {
        std::deque<Job> queue;
        std::mutex locker;

        inline void push_back(const Job& item)
        {
            std::scoped_lock lock(locker);
            queue.push_back(item);
        }
        inline bool pop_front(Job& item)
        {
            std::scoped_lock lock(locker);
            if (queue.empty())
            {
                return false;
            }
            item = std::move(queue.front());
            queue.pop_front();
            return true;
        }
    };

    struct PriorityResources
    {
        uint32_t numThreads = 0;
        Vector<std::thread> threads;
        std::unique_ptr<JobQueue[]> jobQueuePerThread;
        std::atomic<uint32_t> nextQueue{ 0 };
        std::condition_variable wakeCondition;
        std::mutex wakeMutex;

        // Start working on a job queue
        //	After the job queue is finished, it can switch to an other queue and steal jobs from there
        inline void work(uint32_t startingQueue)
        {
            Job job;
            for (uint32_t i = 0; i < numThreads; ++i)
            {
                JobQueue& job_queue = jobQueuePerThread[startingQueue % numThreads];
                while (job_queue.pop_front(job))
                {
                    job.Execute();
                }
                startingQueue++; // go to next queue
            }
        }
    };

    // This structure is responsible to stop worker thread loops.
    // Once this is destroyed, worker threads will be woken up and end their loops.
    struct InternalState final
    {
        uint32_t numCores = 0;
        PriorityResources resources[int(Priority::Count)];
        std::atomic_bool alive{ true };

        void Shutdown()
        {
            if (!numCores)
                return;

            alive.store(false); // indicate that new jobs cannot be started from this point
            bool wake_loop = true;
            std::thread waker([&] {
                while (wake_loop)
                {
                    for (auto& x : resources)
                    {
                        x.wakeCondition.notify_all(); // wakes up sleeping worker threads
                    }
                }
                });
            for (auto& x : resources)
            {
                for (auto& thread : x.threads)
                {
                    thread.join();
                }
            }
            wake_loop = false;
            waker.join();
            for (auto& x : resources)
            {
                x.jobQueuePerThread.reset();
                x.threads.clear();
                x.numThreads = 0;
            }
            numCores = 0;
        }

        ~InternalState()
        {
            Shutdown();
        }

    } static internal_state;

    void Initialize(uint32_t maxThreadCount)
    {
        if (internal_state.numCores > 0)
            return;
        maxThreadCount = std::max(1u, maxThreadCount);

        //Stopwatch timer;
        //std::chrono::high_resolution_clock::time_point timer = std::chrono::high_resolution_clock::now();

        // Retrieve the number of hardware threads in this system:
        internal_state.numCores = std::thread::hardware_concurrency();

        for (int prio = 0; prio < int(Priority::Count); ++prio)
        {
            const Priority priority = (Priority)prio;
            PriorityResources& res = internal_state.resources[prio];

            // Calculate the actual number of worker threads we want:
            switch (priority)
            {
                case Priority::High:
                    res.numThreads = internal_state.numCores - 1; // -1 for main thread
                    break;
                case Priority::Low:
                    res.numThreads = internal_state.numCores - 2; // -1 for main thread, -1 for streaming
                    break;
                case Priority::Streaming:
                    res.numThreads = 1;
                    break;
                default:
                    assert(0);
                    break;
            }
            res.numThreads = Clamp(res.numThreads, 1u, maxThreadCount);
            res.jobQueuePerThread.reset(new JobQueue[res.numThreads]);
            res.threads.reserve(res.numThreads);

            for (uint32_t threadID = 0; threadID < res.numThreads; ++threadID)
            {
#if ALIMER_PLATFORM_LINUX
                std::thread& worker = res.threads.emplace_back([threadID, priority, &res] {

                    switch (priority) {
                        case Priority::Low:
                        case Priority::Streaming:
                            // from the sched(2) manpage:
                            // In the current [Linux 2.6.23+] implementation, each unit of
                            // difference in the nice values of two processes results in a
                            // factor of 1.25 in the degree to which the scheduler favors
                            // the higher priority process.
                            //
                            // so 3 would mean that other (prio 0) threads are around twice as important
                            if (setpriority(PRIO_PROCESS, 0, 3) != 0)
                            {
                                perror("setpriority");
                            }
                            break;
                        case Priority::High:
                            // nothing to do
                            break;
                        default:
                            assert(0);
                    }
#else
                std::thread& worker = res.threads.emplace_back([threadID, &res] {
#endif
                    while (internal_state.alive.load())
                    {
                        res.work(threadID);

                        // finished with jobs, put to sleep
                        std::unique_lock<std::mutex> lock(res.wakeMutex);
                        res.wakeCondition.wait(lock);
                    }

                    });

                auto handle = worker.native_handle();

                int core = threadID + 1; // put threads on increasing cores starting from 2nd
                if (priority == Priority::Streaming)
                {
                    // Put streaming to last core:
                    core = internal_state.numCores - 1 - threadID;
                }

#if defined(_WIN32)
                // Do Windows-specific thread setup:

                // Put each thread on to dedicated core:
                DWORD_PTR affinityMask = 1ull << core;
                DWORD_PTR affinity_result = SetThreadAffinityMask(handle, affinityMask);
                assert(affinity_result > 0);

                if (priority == Priority::High)
                {
                    BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_NORMAL);
                    assert(priority_result != 0);

                    std::wstring wthreadname = L"wi::job_" + std::to_wstring(threadID);
                    HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
                    assert(SUCCEEDED(hr));
                }
                else if (priority == Priority::Low)
                {
                    BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_LOWEST);
                    assert(priority_result != 0);

                    std::wstring wthreadname = L"wi::job_lo_" + std::to_wstring(threadID);
                    HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
                    assert(SUCCEEDED(hr));
                }
                else if (priority == Priority::Streaming)
                {
                    BOOL priority_result = SetThreadPriority(handle, THREAD_PRIORITY_LOWEST);
                    assert(priority_result != 0);

                    std::wstring wthreadname = L"wi::job_st_" + std::to_wstring(threadID);
                    HRESULT hr = SetThreadDescription(handle, wthreadname.c_str());
                    assert(SUCCEEDED(hr));
                }

#elif ALIMER_PLATFORM_LINUX
#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); } while (0)

                int ret;
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                size_t cpusetsize = sizeof(cpuset);

                CPU_SET(core, &cpuset);
                ret = pthread_setaffinity_np(handle, cpusetsize, &cpuset);
                if (ret != 0)
                    handle_error_en(ret, std::string(" pthread_setaffinity_np[" + std::to_string(threadID) + ']').c_str());


                if (priority == Priority::High)
                {
                    std::string thread_name = "wi::job_" + std::to_string(threadID);
                    ret = pthread_setname_np(handle, thread_name.c_str());
                    if (ret != 0)
                        handle_error_en(ret, std::string(" pthread_setname_np[" + std::to_string(threadID) + ']').c_str());
                }
                else if (priority == Priority::Low)
                {
                    std::string thread_name = "wi::job_lo_" + std::to_string(threadID);
                    ret = pthread_setname_np(handle, thread_name.c_str());
                    if (ret != 0)
                        handle_error_en(ret, std::string(" pthread_setname_np[" + std::to_string(threadID) + ']').c_str());
                    // priority is set in the worker function
                }
                else if (priority == Priority::Streaming)
                {
                    std::string thread_name = "wi::job_st_" + std::to_string(threadID);
                    ret = pthread_setname_np(handle, thread_name.c_str());
                    if (ret != 0)
                        handle_error_en(ret, std::string(" pthread_setname_np[" + std::to_string(threadID) + ']').c_str());
                    // priority is set in the worker function
                }

#undef handle_error_en
#endif // _WIN32
            }
        }

        //double elapsedMilliseconds = elapsed_seconds() * 1000.0;

        LOGI("JobSystem: Initialized with {} cores in {} ms\n\tHigh priority threads: {}\n\tLow priority threads: {}\n\tStreaming threads: {}",
            internal_state.numCores,
            0, //timer.elapsed(),
            GetThreadCount(Priority::High),
            GetThreadCount(Priority::Low),
            GetThreadCount(Priority::Streaming));
    }

    void Shutdown()
    {
        internal_state.Shutdown();
    }

    uint32_t GetThreadCount(Priority priority)
    {
        return internal_state.resources[ecast(priority)].numThreads;
    }

    void Execute(Context & ctx, const std::function<void(JobArgs)>&task)
    {
        PriorityResources& res = internal_state.resources[int(ctx.priority)];

        // Context state is updated:
        AtomicAdd(&ctx.counter, 1);

        Job job;
        job.ctx = &ctx;
        job.task = task;
        job.groupID = 0;
        job.groupJobOffset = 0;
        job.groupJobEnd = 1;
        job.sharedMemorySize = 0;

        if (res.numThreads < 1)
        {
            // If job system is not yet initialized, job will be executed immediately here instead of thread:
            job.Execute();
            return;
        }

        res.jobQueuePerThread[res.nextQueue.fetch_add(1) % res.numThreads].push_back(job);
        res.wakeCondition.notify_one();
    }

    void Dispatch(Context & ctx, uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>&task, size_t sharedMemorySize)
    {
        if (jobCount == 0 || groupSize == 0)
        {
            return;
        }
        PriorityResources& res = internal_state.resources[ecast(ctx.priority)];

        const uint32_t groupCount = DispatchGroupCount(jobCount, groupSize);

        // Context state is updated:
        AtomicAdd(&ctx.counter, groupCount);

        Job job;
        job.ctx = &ctx;
        job.task = task;
        job.sharedMemorySize = (uint32_t)sharedMemorySize;

        for (uint32_t groupID = 0; groupID < groupCount; ++groupID)
        {
            // For each group, generate one real job:
            job.groupID = groupID;
            job.groupJobOffset = groupID * groupSize;
            job.groupJobEnd = std::min(job.groupJobOffset + groupSize, jobCount);

            if (res.numThreads < 1)
            {
                // If job system is not yet initialized, job will be executed immediately here instead of thread:
                job.Execute();
            }
            else
            {
                res.jobQueuePerThread[res.nextQueue.fetch_add(1) % res.numThreads].push_back(job);
            }
        }

        if (res.numThreads > 1)
        {
            res.wakeCondition.notify_all();
        }
    }

    uint32_t DispatchGroupCount(uint32_t jobCount, uint32_t groupSize)
    {
        // Calculate the amount of job groups to dispatch (overestimate, or "ceil"):
        return (jobCount + groupSize - 1) / groupSize;
    }

    bool IsBusy(const Context & ctx)
    {
        // Whenever the context label is greater than zero, it means that there is still work that needs to be done
        return AtomicLoad(&ctx.counter) > 0;
    }

    void Wait(const Context & ctx)
    {
        if (!IsBusy(ctx))
            return;

        PriorityResources& res = internal_state.resources[int(ctx.priority)];

        // Wake any threads that might be sleeping:
        res.wakeCondition.notify_all();

        // work() will pick up any jobs that are on stand by and execute them on this thread:
        res.work(res.nextQueue.fetch_add(1) % res.numThreads);

        while (IsBusy(ctx))
        {
            // If we are here, then there are still remaining jobs that work() couldn't pick up.
            //	In this case those jobs are not standing by on a queue but currently executing
            //	on other threads, so they cannot be picked up by this thread.
            //	Allow to swap out this thread by OS to not spin endlessly for nothing
            std::this_thread::yield();
        }
    }
}
