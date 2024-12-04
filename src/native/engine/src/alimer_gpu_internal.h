// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "alimer_internal.h"
#include "alimer_gpu.h"
#include <atomic>

class GPUResource
{
protected:
    GPUResource() = default;
    virtual ~GPUResource() = default;

public:
    // Non-copyable and non-movable
    GPUResource(const GPUResource&) = delete;
    GPUResource& operator=(const GPUResource&) = delete;
    GPUResource(GPUResource&&) = delete;
    GPUResource& operator=(GPUResource&&) = delete;

    virtual uint32_t AddRef()
    {
        return ++_refCount;
    }

    virtual uint32_t Release()
    {
        uint32_t result = --_refCount;
        if (result == 0) {
            delete this;
        }
        return result;
    }

    virtual void SetLabel([[maybe_unused]] const char* label)
    {
    }

private:
    std::atomic<uint32_t> _refCount = 1;
};

//////////////////////////////////////////////////////////////////////////
// RefPtr
// Mostly a copy of Microsoft::WRL::ComPtr<T>
//////////////////////////////////////////////////////////////////////////

template <typename T>
class RefPtr
{
public:
    using InterfaceType = T;

protected:
    InterfaceType* ptr_;
    template<class U> friend class RefPtr;

    void InternalAddRef() const noexcept
    {
        if (ptr_ != nullptr)
        {
            ptr_->AddRef();
        }
    }

    uint32_t InternalRelease() noexcept
    {
        uint32_t ref = 0;
        T* temp = ptr_;

        if (temp != nullptr)
        {
            ptr_ = nullptr;
            ref = temp->Release();
        }

        return ref;
    }

public:
    RefPtr() noexcept
        : ptr_(nullptr)
    {
    }

    RefPtr(std::nullptr_t) noexcept
        : ptr_(nullptr)
    {
    }

    template<class U>
    RefPtr(U* other) noexcept
        : ptr_(other)
    {
        InternalAddRef();
    }

    RefPtr(const RefPtr& other) noexcept
        : ptr_(other.ptr_)
    {
        InternalAddRef();
    }

    // copy ctor that allows to instanatiate class when U* is convertible to T*
    template<class U>
    RefPtr(const RefPtr<U>& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept
        : ptr_(other.ptr_)
    {
        InternalAddRef();
    }

    RefPtr(RefPtr&& other) noexcept
        : ptr_(nullptr)
    {
        if (this != reinterpret_cast<RefPtr*>(&reinterpret_cast<uint8_t&>(other)))
        {
            Swap(other);
        }
    }

    // Move ctor that allows instantiation of a class when U* is convertible to T*
    template<class U>
    RefPtr(RefPtr<U>&& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept
        : ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
    }

    ~RefPtr() noexcept
    {
        InternalRelease();
    }

    RefPtr& operator=(std::nullptr_t) noexcept
    {
        InternalRelease();
        return *this;
    }

    RefPtr& operator=(T* other) noexcept
    {
        if (ptr_ != other)
        {
            RefPtr(other).Swap(*this);
        }
        return *this;
    }

    template <typename U>
    RefPtr& operator=(U* other) noexcept
    {
        RefPtr(other).Swap(*this);
        return *this;
    }

    RefPtr& operator=(const RefPtr& other) noexcept  // NOLINT(bugprone-unhandled-self-assignment)
    {
        if (ptr_ != other.ptr_)
        {
            RefPtr(other).Swap(*this);
        }
        return *this;
    }

    template<class U>
    RefPtr& operator=(const RefPtr<U>& other) noexcept
    {
        RefPtr(other).Swap(*this);
        return *this;
    }

    RefPtr& operator=(RefPtr&& other) noexcept
    {
        RefPtr(static_cast<RefPtr&&>(other)).Swap(*this);
        return *this;
    }

    template<class U>
    RefPtr& operator=(RefPtr<U>&& other) noexcept
    {
        RefPtr(static_cast<RefPtr<U>&&>(other)).Swap(*this);
        return *this;
    }

    void Swap(RefPtr&& r) noexcept
    {
        T* tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
    }

    void Swap(RefPtr& r) noexcept
    {
        T* tmp = ptr_;
        ptr_ = r.ptr_;
        r.ptr_ = tmp;
    }

    [[nodiscard]] T* Get() const noexcept
    {
        return ptr_;
    }

    operator T* () const
    {
        return ptr_;
    }

    InterfaceType* operator->() const noexcept
    {
        return ptr_;
    }

    T** operator&()   // NOLINT(google-runtime-operator)
    {
        return &ptr_;
    }

    [[nodiscard]] T* const* GetAddressOf() const noexcept
    {
        return &ptr_;
    }

    [[nodiscard]] T** GetAddressOf() noexcept
    {
        return &ptr_;
    }

    [[nodiscard]] T** ReleaseAndGetAddressOf() noexcept
    {
        InternalRelease();
        return &ptr_;
    }

    T* Detach() noexcept
    {
        T* ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

    // Set the pointer while keeping the object's reference count unchanged
    void Attach(InterfaceType* other)
    {
        if (ptr_ != nullptr)
        {
            auto ref = ptr_->Release();
            (void)ref;

            // Attaching to the same object only works if duplicate references are being coalesced. Otherwise
            // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
            assert(ref != 0 || ptr_ != other);
        }

        ptr_ = other;
    }

    // Create a wrapper around a raw object while keeping the object's reference count unchanged
    static RefPtr<T> Create(T* other)
    {
        RefPtr<T> Ptr;
        Ptr.Attach(other);
        return Ptr;
    }

    uint32_t Reset()
    {
        return InternalRelease();
    }
};

struct GPUBufferImpl : public GPUResource
{

};

struct GPUTextureImpl : public GPUResource
{

};

struct GPUTextureViewImpl : public GPUResource
{

};

struct GPUShaderModuleImpl : public GPUResource
{

};


struct GPUCommandBufferImpl : public GPUResource
{

};

struct GPUQueueImpl : public GPUResource
{
    virtual GPUCommandBuffer CreateCommandBuffer(const GPUCommandBufferDescriptor* descriptor) = 0;
};

struct GPUDeviceImpl : public GPUResource
{
    virtual GPUQueue GetQueue(GPUQueueType type) = 0;
    virtual uint64_t CommitFrame() = 0;

    /* Resource creation */
    virtual GPUBuffer CreateBuffer(const GPUBufferDescriptor* descriptor, const void* pInitialData) = 0;
};

struct GPUSurfaceImpl : public GPUResource
{
};

struct GPUAdapterImpl : public GPUResource
{
    virtual GPUResult GetLimits(GPULimits* limits) const = 0;
    virtual GPUDevice CreateDevice() = 0;
};

struct GPUInstance : public GPUResource
{
public:
    virtual GPUSurface CreateSurface(Window* window) = 0;
    virtual GPUAdapter RequestAdapter(const GPURequestAdapterOptions* options) = 0;
};

#if defined(ALIMER_GPU_VULKAN)
_ALIMER_EXTERN bool Vulkan_IsSupported(void);
_ALIMER_EXTERN GPUInstance* Vulkan_CreateInstance(const GPUConfig* config);
#endif

#if defined(ALIMER_GPU_WEBGPU)
_ALIMER_EXTERN bool WGPU_IsSupported(void);
_ALIMER_EXTERN GPUInstance* WGPU_CreateInstance(const GPUConfig* config);
#endif
