// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.
// This is C# port of VulkanMemoryAllocator (https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/LICENSE.txt)

namespace Vortice.Vulkan;

internal partial struct VmaMappingHysteresis
{
    private const int COUNTER_MIN_EXTRA_MAPPING = 7;

    private uint _minorCounter = 0;
    private uint _majorCounter = 0;

    public VmaMappingHysteresis()
    {

    }

    public uint ExtraMapping { get; private set; }

    // Call when Map was called.
    // Returns true if switched to extra +1 mapping reference count.
    public bool PostMap()
    {
        if (ExtraMapping == 0)
        {
            ++_majorCounter;
            if (_majorCounter >= COUNTER_MIN_EXTRA_MAPPING)
            {
                ExtraMapping = 1;
                _majorCounter = 0;
                _minorCounter = 0;
                return true;
            }
        }
        else // m_ExtraMapping == 1
        {
            PostMinorCounter();

        }

        return false;
    }

    // Call when Unmap was called.
    public void PostUnmap()
    {
        if (ExtraMapping == 0)
            ++_majorCounter;
        else // m_ExtraMapping == 1
            PostMinorCounter();
    }

    // Call when allocation was made from the memory block.
    public void PostAlloc()
    {
        if (ExtraMapping == 1)
            ++_majorCounter;
        else // m_ExtraMapping == 0
            PostMinorCounter();
    }

    // Call when allocation was freed from the memory block.
    // Returns true if switched to extra -1 mapping reference count.
    public bool PostFree()
    {
        if (ExtraMapping == 1)
        {
            ++_majorCounter;
            if (_majorCounter >= COUNTER_MIN_EXTRA_MAPPING &&
                _majorCounter > _minorCounter + 1)
            {
                ExtraMapping = 0;
                _majorCounter = 0;
                _minorCounter = 0;
                return true;
            }
        }
        else // m_ExtraMapping == 0
        {
            PostMinorCounter();
        }

        return false;
    }


    private void PostMinorCounter()
    {
        if (_minorCounter < _majorCounter)
        {
            ++_minorCounter;
        }
        else if (_majorCounter > 0)
        {
            --_majorCounter;
            --_minorCounter;
        }
    }
}
