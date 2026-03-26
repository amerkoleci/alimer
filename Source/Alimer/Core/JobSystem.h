// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// Code used from WickedEngine: https://github.com/turanszkij/WickedEngine/blob/master/WickedEngine/wiJobSystem.h

#pragma once

#include "Alimer/Core/Base.h"
#include <atomic>
#include <functional>

namespace Alimer::JobSystem
{
    static constexpr uint32_t kSmallSubtaskGroupSize = 64;

    enum class Priority : uint8_t
    {
        High,		// Default
        Low,		// Pool of low priority threads, useful for generic tasks that shouldn't interfere with high priority tasks
        Streaming,	// Single low priority thread, for streaming resources
        Count
    };

    struct JobArgs
    {
        uint32_t jobIndex;		// job index relative to dispatch (like SV_DispatchThreadID in HLSL)
        uint32_t groupID;		// group index relative to dispatch (like SV_GroupID in HLSL)
        uint32_t groupIndex;	// job index relative to group (like SV_GroupIndex in HLSL)
        bool isFirstJobInGroup;	// is the current job the first one in the group?
        bool isLastJobInGroup;	// is the current job the last one in the group?
        void* sharedMemory;		// stack memory shared within the current group (jobs within a group execute serially)
    };

    // Defines a state of execution, can be waited on
    struct Context
    {
        volatile long counter{ 0 };
        Priority priority = Priority::High;
    };

    void Initialize(uint32_t maxThreadCount = ~0u);
    void Shutdown();

    /// Get the job system thread count.
    uint32_t GetThreadCount(Priority priority = Priority::High);

    /// Add a task to execute asynchronously. Any idle thread will execute this.
    void Execute(Context& ctx, const std::function<void(JobArgs)>& task);

    // Divide a task onto multiple jobs and execute in parallel.
    //	jobCount	: how many jobs to generate for this task.
    //	groupSize	: how many jobs to execute per thread. Jobs inside a group execute serially. It might be worth to increase for small jobs
    //	task		: receives a JobArgs as parameter
    void Dispatch(Context& ctx, uint32_t jobCount, uint32_t groupSize, const std::function<void(JobArgs)>& task, size_t sharedMemorySize = 0);

    /// Returns the amount of job groups that will be created for a set number of jobs and group size
    uint32_t DispatchGroupCount(uint32_t jobCount, uint32_t groupSize);

    /// Check if any threads are working currently or not
    bool IsBusy(const Context& ctx);

    /// Wait until all threads become idle, current thread will become a worker thread, executing jobs
    void Wait(const Context& ctx);
}
