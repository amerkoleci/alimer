// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Core/Types.h"
#include <atomic>

namespace Alimer
{
    /// Reference count structure.
    struct RefCount
    {
        /// Construct.
        RefCount() = default;

        /// Destruct.
        ~RefCount()
        {
            // Set reference counts below zero to fire asserts if this object is still accessed
            refs = -1;
            weakRefs = -1;
        }

        /// Reference count. If below zero, the object has been destroyed.
        std::atomic_uint32_t refs = 0;
        /// Weak reference count.
        std::atomic_uint32_t weakRefs = 0;
    };

    /// Base class for intrusively reference counted objects that can be pointed to with RefPtr. These are noncopyable and non-assignable.
    class ALIMER_API RefCounted
    {
    protected:
        RefCounted() ;

    public:
        virtual ~RefCounted();

        virtual uint32_t AddRef();
        virtual uint32_t Release();

        // Non-copyable and non-movable
        RefCounted(const RefCounted&) = delete;
        RefCounted(const RefCounted&&) = delete;
        RefCounted& operator=(const RefCounted&) = delete;
        RefCounted& operator=(const RefCounted&&) = delete;

    private:
        /// Pointer to the reference count structure.
        RefCount* refCount = nullptr;
    };

    // Type alias for intrusive_ptr template
    template <typename T> class RefPtr
    {
    protected:
        T* ptr_;
        template <class U> friend class RefPtr;

        void AddRef() const noexcept
        {
            if (ptr_ != nullptr)
            {
                ptr_->AddRef();
            }
        }

        void ReleaseRef() noexcept
        {
            if (ptr_)
            {
                ptr_->Release();
                ptr_ = nullptr;
            }
        }

    public:
        /// Construct a null shared pointer.
        RefPtr() noexcept
            : ptr_(nullptr)
        {
        }

        /// Construct a null shared pointer.
        constexpr RefPtr(std::nullptr_t) noexcept
            : ptr_(nullptr)
        {
        }

        /// Construct from a raw pointer.
        template<class U>
        RefPtr(_In_opt_ U* other) noexcept
            : ptr_(other)
        {
            AddRef();
        }

        /// Copy-construct from another shared pointer.
        RefPtr(const RefPtr<T>& other) noexcept
            : ptr_(other.ptr_)
        {
            AddRef();
        }

        /// Move-construct from another shared pointer.
        RefPtr(RefPtr<T>&& other) noexcept
            : ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        /// Copy-construct from another shared pointer allowing implicit upcasting.
        template <class U> RefPtr(const RefPtr<U>& rhs) noexcept
            : ptr_(rhs.ptr_)
        {
            static_assert(std::is_convertible<U*, T*>::value, "");
            AddRef();
        }

        /// Destruct. Release the object reference.
        ~RefPtr() noexcept
        {
            ReleaseRef();
        }

        RefPtr& operator=(std::nullptr_t)
        {
            ReleaseRef();
            return *this;
        }

        /// Assign from another shared pointer.
        RefPtr<T>& operator=(const RefPtr<T>& other)
        {
            if (&other == this)
                return *this;

            other.AddRef();
            ReleaseRef();
            ptr_ = other.ptr_;
            return *this;
        }

        /// Assign from another shared pointer allowing implicit upcasting.
        template <class U> RefPtr<T>& operator=(const RefPtr<U>& other)
        {
            static_assert(std::is_convertible<U*, T*>::value, "");

            other.AddRef();
            ReleaseRef();
            ptr_ = other.ptr_;
            return *this;
        }

        /// Move-assign from another shared pointer.
        RefPtr<T>& operator=(RefPtr<T>&& other)
        {
            if (&other == this)
                return *this;

            ReleaseRef();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            return *this;
        }

        template <typename U>
        RefPtr<T>& operator=(RefPtr<U>&& other)
        {
            static_assert(std::is_convertible<U*, T*>::value, "");

            ReleaseRef();
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
            return *this;
        }

        /// Swap with another RefPtr.
        void Swap(_Inout_ RefPtr& rhs) { std::swap(ptr_, rhs.ptr_); }

        explicit operator bool() const { return ptr_ != nullptr; }

        /// Return the raw pointer.
        T* Get() const { return ptr_; }

        /// Point to the object.
        T* operator->() const { return ptr_; }

        /// Dereference the object.
        T& operator*() const { return *ptr_; }

        /// Convert to a raw pointer.
        operator T* () const { return ptr_; }

        /// Subscript the object if applicable.
        T& operator[](size_t index)
        {
            ALIMER_ASSERT(ptr_);
            return ptr_[index];
        }

        /// Test for less than with another shared pointer.
        template <class U> bool operator<(const RefPtr<U>& rhs) const { return ptr_ < rhs.ptr_; }

        /// Test for equality with another shared pointer.
        template <class U> bool operator==(const RefPtr<U>& rhs) const { return ptr_ == rhs.ptr_; }

        /// Test for inequality with another shared pointer.
        template <class U> bool operator!=(const RefPtr<U>& rhs) const { return ptr_ != rhs.ptr_; }

        /// Check if the pointer is null.
        bool IsNull() const { return ptr_ == nullptr; }

        /// Check if the pointer is not null.
        bool IsNotNull() const { return ptr_ != nullptr; }

        T* const* GetAddressOf() const noexcept { return &ptr_; }
        T** GetAddressOf() noexcept { return &ptr_; }

        T** ReleaseAndGetAddressOf() noexcept
        {
            ReleaseRef();
            return &ptr_;
        }

        T* Detach() noexcept
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        /// Reset with another pointer.
        void Reset(T* ptr = nullptr)
        {
            RefPtr<T> copy(ptr);
            Swap(copy);
        }

        /// Perform a static cast from a shared pointer of another type.
        template <class U> void StaticCast(const RefPtr<U>& rhs)
        {
            RefPtr<T> copy(static_cast<T*>(rhs.Get()));
            Swap(copy);
        }

        /// Perform a dynamic cast from a shared pointer of another type.
        template <class U> void DynamicCast(const RefPtr<U>& rhs)
        {
            RefPtr<T> copy(dynamic_cast<T*>(rhs.Get()));
            Swap(copy);
        }
    };

    /// Perform a static cast from one shared pointer type to another.
    template <class T, class U> RefPtr<T> StaticCast(const RefPtr<U>& ptr)
    {
        RefPtr<T> ret;
        ret.StaticCast(ptr);
        return ret;
    }

    /// Perform a dynamic cast from one weak pointer type to another.
    template <class T, class U> RefPtr<T> DynamicCast(const RefPtr<U>& ptr)
    {
        RefPtr<T> ret;
        ret.DynamicCast(ptr);
        return ret;
    }
}
