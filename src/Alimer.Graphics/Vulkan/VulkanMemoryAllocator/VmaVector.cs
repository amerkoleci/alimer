// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;
using System.Runtime.CompilerServices;
using static Vortice.Vulkan.VmaUtils;

namespace Vortice.Vulkan;

internal unsafe partial struct VmaVector<T> : IDisposable
    where T : unmanaged
{
    private T* m_pArray;

    private nuint m_Count;

    private nuint m_Capacity;

    public VmaVector()
    {
    }

    public VmaVector(nuint count)
    {
        m_pArray = (count != 0) ? AllocateArray<T>(count) : null;
        m_Count = count;
        m_Capacity = count;
    }

    public VmaVector(in VmaVector<T> src)
    {
        m_pArray = (src.m_Count != 0) ? AllocateArray<T>(src.m_Count) : null;
        m_Count = src.m_Count;
        m_Capacity = src.m_Count;

        if (m_Count > 0)
        {
            Unsafe.CopyBlock(m_pArray, src.m_pArray, (uint)(m_Count * __sizeof<T>()));
        }
    }

    public void Dispose()
    {
        Free(m_pArray);
    }

    public readonly bool empty()
    {
        return m_Count == 0;
    }

    public readonly nuint size()
    {
        return m_Count;
    }

    public T* data()
    {
        return m_pArray;
    }

    public void clear(bool freeMemory = false)
    {
        resize(0, freeMemory);
    }

    public T* begin()
    {
        return m_pArray;
    }

    public T* end()
    {
        return m_pArray + m_Count;
    }

    public T* rend()
    {
        return begin() - 1;
    }

    public T* rbegin()
    {
        return end() - 1;
    }

    public readonly T* cbegin()
    {
        return m_pArray;
    }

    public readonly T* cend()
    {
        return m_pArray + m_Count;
    }

    public readonly T* crbegin()
    {
        return cend() - 1;
    }

    public readonly T* crend()
    {
        return cbegin() - 1;
    }

    public void push_front(in T src)
    {
        insert(0, src);
    }

    public void push_back(in T src)
    {
        nuint newIndex = size();
        resize(newIndex + 1);
        m_pArray[newIndex] = src;
    }

    public void pop_front()
    {
        Debug.Assert(m_Count > 0);
        remove(0);
    }

    public void pop_back()
    {
        Debug.Assert(m_Count > 0);
        resize(size() - 1);
    }

    public ref T front()
    {
        Debug.Assert(m_Count > 0);
        return ref m_pArray[0];
    }

    public ref T back()
    {
        Debug.Assert(m_Count > 0);
        return ref m_pArray[m_Count - 1];
    }

    public void reserve(nuint newCapacity, bool freeMemory = false)
    {
        newCapacity = Math.Max(newCapacity, m_Count);

        if ((newCapacity < m_Capacity) && !freeMemory)
        {
            newCapacity = m_Capacity;
        }

        if (newCapacity != m_Capacity)
        {
            T* newArray = (newCapacity != 0) ? AllocateArray<T>(newCapacity) : null;

            if (m_Count != 0)
            {
                Unsafe.CopyBlock(newArray, m_pArray, (uint)(m_Count * __sizeof<T>()));
            }
            Free(m_pArray);

            m_Capacity = newCapacity;
            m_pArray = newArray;
        }
    }

    public void resize(nuint newCount, bool freeMemory = false)
    {
        nuint newCapacity = m_Capacity;

        if (newCount > m_Capacity)
        {
            newCapacity = Math.Max(newCount, Math.Max(m_Capacity * 3 / 2, 8));
        }
        else if (freeMemory)
        {
            newCapacity = newCount;
        }

        if (newCapacity != m_Capacity)
        {
            T* newArray = (newCapacity != 0) ? AllocateArray<T>(newCapacity) : null;
            nuint elementsToCopy = Math.Min(m_Count, newCount);

            if (elementsToCopy != 0)
            {
                Unsafe.CopyBlock(newArray, m_pArray, (uint)(elementsToCopy * __sizeof<T>()));
            }
            Free(m_pArray);

            m_Capacity = newCapacity;
            m_pArray = newArray;
        }

        m_Count = newCount;
    }

    public void insert(nuint index, in T src)
    {
        Debug.Assert(index <= m_Count);

        nuint oldCount = size();
        resize(oldCount + 1);

        if (index < oldCount)
        {
            _ = memmove(m_pArray + (index + 1), m_pArray + index, (oldCount - index) * __sizeof<T>());
        }
        m_pArray[index] = src;
    }

    public void remove(nuint index)
    {
        Debug.Assert(index < m_Count);
        nuint oldCount = size();

        if (index < oldCount - 1)
        {
            _ = memmove(m_pArray + index, m_pArray + (index + 1), (oldCount - index - 1) * __sizeof<T>());
        }
        resize(oldCount - 1);
    }

    public ref T this[nuint index]
    {
        get
        {
            Debug.Assert(index < m_Count);
            return ref m_pArray[index];
        }
    }
}

