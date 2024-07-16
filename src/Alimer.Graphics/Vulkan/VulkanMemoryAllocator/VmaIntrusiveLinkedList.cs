// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

using System.Diagnostics;

namespace Vortice.Vulkan;

internal unsafe interface VmaItemTypeTraits<ItemTypeTraits, ItemType>
    where ItemTypeTraits : unmanaged, VmaItemTypeTraits<ItemTypeTraits, ItemType>
    where ItemType : unmanaged
{
    static abstract ItemType* GetPrev(ItemType* item);

    static abstract ItemType* GetNext(ItemType* item);

    static abstract ref ItemType* AccessPrev(ItemType* item);

    static abstract ref ItemType* AccessNext(ItemType* item);
}


internal unsafe partial struct VmaIntrusiveLinkedList<ItemTypeTraits, ItemType> : IDisposable
    where ItemTypeTraits : unmanaged, VmaItemTypeTraits<ItemTypeTraits, ItemType>
    where ItemType : unmanaged
{
    private ItemType* m_Front;

    private ItemType* m_Back;

    private nuint m_Count;

    public static ItemType* GetPrev(ItemType* item)
    {
        return ItemTypeTraits.GetPrev(item);
    }

    public static ItemType* GetNext(ItemType* item)
    {
        return ItemTypeTraits.GetNext(item);
    }

    public VmaIntrusiveLinkedList(ref VmaIntrusiveLinkedList<ItemTypeTraits, ItemType>* src)
    {
        m_Front = (*src).m_Front;
        m_Back = (*src).m_Back;
        m_Count = (*src).m_Count;

        (*src).m_Front = (*src).m_Back = null;
        (*src).m_Count = 0;
    }

    public void Dispose()
    {
        Debug.Assert(IsEmpty());
    }

    public readonly nuint GetCount()
    {
        return m_Count;
    }

    public readonly bool IsEmpty()
    {
        return m_Count == 0;
    }

    public ItemType* Front()
    {
        return m_Front;
    }

    public ItemType* Back()
    {
        return m_Back;
    }

    public void PushBack(ItemType* item)
    {
        Debug.Assert((ItemTypeTraits.GetPrev(item) == null) && (ItemTypeTraits.GetNext(item) == null));

        if (IsEmpty())
        {
            m_Front = item;
            m_Back = item;
            m_Count = 1;
        }
        else
        {
            ItemTypeTraits.AccessPrev(item) = m_Back;
            ItemTypeTraits.AccessNext(m_Back) = item;

            m_Back = item;
            ++m_Count;
        }
    }

    public void PushFront(ItemType* item)
    {
        Debug.Assert((ItemTypeTraits.GetPrev(item) == null) && (ItemTypeTraits.GetNext(item) == null));

        if (IsEmpty())
        {
            m_Front = item;
            m_Back = item;
            m_Count = 1;
        }
        else
        {
            ItemTypeTraits.AccessNext(item) = m_Front;
            ItemTypeTraits.AccessPrev(m_Front) = item;

            m_Front = item;
            ++m_Count;
        }
    }

    public ItemType* PopBack()
    {
        Debug.Assert(m_Count > 0);

        ItemType* backItem = m_Back;
        ItemType* prevItem = ItemTypeTraits.GetPrev(backItem);

        if (prevItem != null)
        {
            ItemTypeTraits.AccessNext(prevItem) = null;
        }

        m_Back = prevItem;
        --m_Count;

        ItemTypeTraits.AccessPrev(backItem) = null;
        ItemTypeTraits.AccessNext(backItem) = null;

        return backItem;
    }

    public ItemType* PopFront()
    {
        Debug.Assert(m_Count > 0);

        ItemType* frontItem = m_Front;
        ItemType* nextItem = ItemTypeTraits.GetNext(frontItem);

        if (nextItem != null)
        {
            ItemTypeTraits.AccessPrev(nextItem) = null;
        }

        m_Front = nextItem;
        --m_Count;

        ItemTypeTraits.AccessPrev(frontItem) = null;
        ItemTypeTraits.AccessNext(frontItem) = null;

        return frontItem;
    }

    // MyItem can be null - it means PushBack.
    public void InsertBefore(ItemType* existingItem, ItemType* newItem)
    {
        Debug.Assert((newItem != null) && (ItemTypeTraits.GetPrev(newItem) == null) && (ItemTypeTraits.GetNext(newItem) == null));

        if (existingItem != null)
        {
            ItemType* prevItem = ItemTypeTraits.GetPrev(existingItem);

            ItemTypeTraits.AccessPrev(newItem) = prevItem;
            ItemTypeTraits.AccessNext(newItem) = existingItem;
            ItemTypeTraits.AccessPrev(existingItem) = newItem;

            if (prevItem != null)
            {
                ItemTypeTraits.AccessNext(prevItem) = newItem;
            }
            else
            {
                Debug.Assert(m_Front == existingItem);
                m_Front = newItem;
            }

            ++m_Count;
        }
        else
        {
            PushBack(newItem);
        }
    }

    // MyItem can be null - it means PushFront.
    public void InsertAfter(ItemType* existingItem, ItemType* newItem)
    {
        Debug.Assert((newItem != null) && (ItemTypeTraits.GetPrev(newItem) == null) && (ItemTypeTraits.GetNext(newItem) == null));

        if (existingItem != null)
        {
            ItemType* nextItem = ItemTypeTraits.GetNext(existingItem);

            ItemTypeTraits.AccessNext(newItem) = nextItem;
            ItemTypeTraits.AccessPrev(newItem) = existingItem;
            ItemTypeTraits.AccessNext(existingItem) = newItem;

            if (nextItem != null)
            {
                ItemTypeTraits.AccessPrev(nextItem) = newItem;
            }
            else
            {
                Debug.Assert(m_Back == existingItem);
                m_Back = newItem;
            }

            ++m_Count;
        }
        else
        {
            PushFront(newItem);
        }
    }

    public void Remove(ItemType* item)
    {
        Debug.Assert((item != null) && (m_Count > 0));

        if (ItemTypeTraits.GetPrev(item) != null)
        {
            ItemTypeTraits.AccessNext(ItemTypeTraits.AccessPrev(item)) = ItemTypeTraits.GetNext(item);
        }
        else
        {
            Debug.Assert(m_Front == item);
            m_Front = ItemTypeTraits.GetNext(item);
        }

        if (ItemTypeTraits.GetNext(item) != null)
        {
            ItemTypeTraits.AccessPrev(ItemTypeTraits.AccessNext(item)) = ItemTypeTraits.GetPrev(item);
        }
        else
        {
            Debug.Assert(m_Back == item);
            m_Back = ItemTypeTraits.GetPrev(item);
        }

        ItemTypeTraits.AccessPrev(item) = null;
        ItemTypeTraits.AccessNext(item) = null;

        --m_Count;
    }

    public void RemoveAll()
    {
        if (!IsEmpty())
        {
            ItemType* item = m_Back;

            while (item != null)
            {
                ItemType* prevItem = ItemTypeTraits.AccessPrev(item);

                ItemTypeTraits.AccessPrev(item) = null;
                ItemTypeTraits.AccessNext(item) = null;

                item = prevItem;
            }

            m_Front = null;
            m_Back = null;
            m_Count = 0;
        }
    }
}

