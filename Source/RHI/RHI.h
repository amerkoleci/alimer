// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <cassert>
#include <atomic>
#include <type_traits>

#define RHI_ENUM_CLASS_FLAG_OPERATORS(T) \
    inline T operator | (T a, T b) { return T(uint32_t(a) | uint32_t(b)); } \
    inline T operator & (T a, T b) { return T(uint32_t(a) & uint32_t(b)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline T operator ~ (T a) { return T(~uint32_t(a)); } /* NOLINT(bugprone-macro-parentheses) */ \
    inline bool operator !(T a) { return uint32_t(a) == 0; } \
    inline bool operator ==(T a, uint32_t b) { return uint32_t(a) == b; } \
    inline bool operator !=(T a, uint32_t b) { return uint32_t(a) != b; }

#if defined(RHI_SHARED_LIBRARY_BUILD)
#   if defined(_MSC_VER)
#       define RHI_API __declspec(dllexport)
#   elif defined(__GNUC__)
#       define RHI_API __attribute__((visibility("default")))
#   else
#       define RHI_API
#       pragma warning "Unknown dynamic link import/export semantics."
#   endif
#elif defined(RHI_SHARED_LIBRARY_INCLUDE)
#   if defined(_MSC_VER)
#       define RHI_API __declspec(dllimport)
#   else
#       define RHI_API
#   endif
#else
#   define RHI_API
#endif

namespace RHI
{
    enum class GraphicsAPI : uint32_t
    {
        Direct3D12,
        Vulkan
    };

    enum class ValidationMode : uint32_t
    {
        /// No validation is enabled.
        Disabled,
        /// Print warnings and errors
        Enabled,
        /// Print all warnings, errors and info messages
        Verbose,
        /// Enable GPU-based validation
        GPU
    };

    class RHI_API IResource
    {
    protected:
        IResource() = default;
        virtual ~IResource() = default;

    public:
        // Non-copyable and non-movable
        IResource(const IResource&) = delete;
        IResource(const IResource&&) = delete;
        IResource& operator=(const IResource&) = delete;
        IResource& operator=(const IResource&&) = delete;

        virtual uint32_t AddRef()
        {
            return ++refCount;
        }

        virtual uint32_t Release()
        {
            uint32_t result = --refCount;
            if (result == 0) {
                delete this;
            }
            return result;
        }

    private:
        std::atomic_uint32_t refCount = 1;
    };

    //////////////////////////////////////////////////////////////////////////
    // RefCountPtr
    // Mostly a copy of Microsoft::WRL::ComPtr<T>
    //////////////////////////////////////////////////////////////////////////

    template <typename T>
    class RefCountPtr
    {
    public:
        using InterfaceType = T;

    protected:
        InterfaceType* ptr_;
        template<class U> friend class RefCountPtr;

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
        RefCountPtr() noexcept : ptr_(nullptr)
        {
        }

        RefCountPtr(std::nullptr_t) noexcept : ptr_(nullptr)
        {
        }

        template<class U>
        RefCountPtr(U* other) noexcept : ptr_(other)
        {
            InternalAddRef();
        }

        RefCountPtr(const RefCountPtr& other) noexcept : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        // copy ctor that allows to instanatiate class when U* is convertible to T*
        template<class U>
        RefCountPtr(const RefCountPtr<U>& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept :
            ptr_(other.ptr_)

        {
            InternalAddRef();
        }

        RefCountPtr(RefCountPtr&& other) noexcept : ptr_(nullptr)
        {
            if (this != reinterpret_cast<RefCountPtr*>(&reinterpret_cast<unsigned char&>(other)))
            {
                Swap(other);
            }
        }

        // Move ctor that allows instantiation of a class when U* is convertible to T*
        template<class U>
        RefCountPtr(RefCountPtr<U>&& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void*>::type* = nullptr) noexcept :
            ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        ~RefCountPtr() noexcept
        {
            InternalRelease();
        }

        RefCountPtr& operator=(std::nullptr_t) noexcept
        {
            InternalRelease();
            return *this;
        }

        RefCountPtr& operator=(T* other) noexcept
        {
            if (ptr_ != other)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        RefCountPtr& operator=(U* other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(const RefCountPtr& other) noexcept  // NOLINT(bugprone-unhandled-self-assignment)
        {
            if (ptr_ != other.ptr_)
            {
                RefCountPtr(other).Swap(*this);
            }
            return *this;
        }

        template<class U>
        RefCountPtr& operator=(const RefCountPtr<U>& other) noexcept
        {
            RefCountPtr(other).Swap(*this);
            return *this;
        }

        RefCountPtr& operator=(RefCountPtr&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr&&>(other)).Swap(*this);
            return *this;
        }

        template<class U>
        RefCountPtr& operator=(RefCountPtr<U>&& other) noexcept
        {
            RefCountPtr(static_cast<RefCountPtr<U>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(RefCountPtr&& r) noexcept
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        void Swap(RefCountPtr& r) noexcept
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
        static RefCountPtr<T> Create(T* other)
        {
            RefCountPtr<T> Ptr;
            Ptr.Attach(other);
            return Ptr;
        }

        uint32_t Reset()
        {
            return InternalRelease();
        }
    };

    class RHI_API ICommandList : public IResource
    {
    public:
    };

    class RHI_API IDevice : public IResource
    {
    public:
    };

    using DeviceHandle = RefCountPtr<IDevice>;

    RHI_API DeviceHandle CreateDevice(GraphicsAPI api, ValidationMode validationMode = ValidationMode::Disabled);
}

#undef RHI_ENUM_CLASS_FLAG_OPERATORS

namespace std
{
}
