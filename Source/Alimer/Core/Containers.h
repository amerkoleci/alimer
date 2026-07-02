// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Core/Base.h"

#include <array>
#if defined(ALIMER_STD_CONTAINERS)
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <span>
#else
#include <vector>
#include <ankerl/unordered_dense.h>
#include <span>
#endif
#include <type_traits>
#include <algorithm>

namespace Alimer
{
    static constexpr uint32_t kInvalidIndex = -1;

    template <typename T, size_t Size> using Array = std::array<T, Size>;

#if defined(ALIMER_STD_CONTAINERS)
    template<typename T, typename Allocator = std::allocator<T>>
    using Vector = std::vector<T, Allocator>;

    template<typename Key>
    using UnorderedSet = std::unordered_set<Key>;

    template<class TKey, class TValue>
    using UnorderedMap = std::unordered_map<TKey, TValue>;

    template <typename T>
    using TSpan = std::span<T>;
#else
    template<typename T, typename Allocator = std::allocator<T>>
    using Vector = std::vector<T, Allocator>;

    template<typename Key>
    using UnorderedSet = ankerl::unordered_dense::set<Key>;

    template<class TKey, class TValue>
    using UnorderedMap = ankerl::unordered_dense::map<TKey, TValue>;

    template <typename T>
    using TSpan = std::span<T>;
#endif

    template<typename TVector, typename V>
    inline void VectorRemove(TVector& vector, const V& Value)
    {
        auto it = std::find(vector.begin(), vector.end(), Value);
        if (it != vector.end())
        {
            vector.erase(it);
        }
    }

    template<typename T>
    inline typename Vector<T>::const_iterator VectorFind(Vector<T> const& vector, T const& value)
    {
        return std::find(vector.begin(), vector.end(), value);
    }

    template<typename T, typename V, typename Predicate>
    inline typename Vector<T>::const_iterator VectorFind(Vector<T> const& vector, V const& value, Predicate predicate)
    {
        return std::find(vector.begin(), vector.end(), value, std::forward<Predicate>(predicate));
    }

    template<typename T>
    inline typename Vector<T>::iterator VectorFind(Vector<T>& vector, T const& value)
    {
        return std::find(vector.begin(), vector.end(), value);
    }

    template<typename T, typename V, typename Predicate>
    inline typename Vector<T>::iterator VectorFind(Vector<T>& vector, V const& value, Predicate predicate)
    {
        return std::find(vector.begin(), vector.end(), value, std::forward<Predicate>(predicate));
    }

    template<typename T, typename V>
    inline bool VectorContains(Vector<T> const& vector, V const& value)
    {
        return std::find(vector.begin(), vector.end(), value) != vector.end();
    }

    // Usage: VectorContains( vector, value, [] ( T const& typeRef, V const& valueRef ) { ... } );
    template<typename T, typename V, typename Predicate>
    inline bool VectorContains(const Vector<T>& vector, V const& value, Predicate predicate)
    {
        return std::find(vector.begin(), vector.end(), value, std::forward<Predicate>(predicate)) != vector.end();
    }

    template<typename T, typename V, typename Predicate>
    inline bool VectorContains(const Vector<T>& vector, Predicate predicate)
    {
        return std::find_if(vector.begin(), vector.end(), std::forward<Predicate>(predicate)) != vector.end();
    }

    template<typename T>
    inline void VectorRemoveAtIndex(Vector<T>& vector, uint32_t index)
    {
        vector.erase(vector.begin() + index);
    }

    template<typename T>
    inline int32_t VectorFindIndex(const Vector<T>& vector, T const& value)
    {
        auto iter = std::find(vector.begin(), vector.end(), value);
        if (iter == vector.end())
        {
            return kInvalidIndex;
        }
        else
        {
            return (int32_t)(iter - vector.begin());
        }
    }

    template<typename T, typename V, typename Predicate>
    inline int32_t VectorFindIndex(const Vector<T>& vector, const V& value, Predicate predicate)
    {
        auto iter = std::find(vector.begin(), vector.end(), value, predicate);
        if (iter == vector.end())
        {
            return kInvalidIndex;
        }
        else
        {
            return (int32_t)(iter - vector.begin());
        }
    }

    template<typename T, typename V, typename Predicate>
    inline int32_t VectorFindIndex(const Vector<T>& vector, Predicate predicate)
    {
        auto iter = std::find_if(vector.begin(), vector.end(), predicate);
        if (iter == vector.end())
        {
            return kInvalidIndex;
        }
        else
        {
            return (int32_t)(iter - vector.begin());
        }
    }
}
