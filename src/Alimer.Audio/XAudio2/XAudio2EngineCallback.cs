// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using TerraFX.Interop.Windows;

namespace Alimer.Audio.XAudio2;

internal unsafe struct XAudio2EngineCallback
{
    public static readonly void** Vtbl = InitVtbl();

    static void** InitVtbl()
    {
        void** lpVtbl = (void**)RuntimeHelpers.AllocateTypeAssociatedMemory(typeof(XAudio2EngineCallback), sizeof(void*) * 3);

        /* OnProcessingPassStart */
        lpVtbl[0] = (delegate* unmanaged<XAudio2EngineCallback*, void>)&OnProcessingPassStart;
        /* OnProcessingPassEnd */
        lpVtbl[1] = (delegate* unmanaged<XAudio2EngineCallback*, void>)&OnProcessingPassEnd;
        /* OnCriticalError */
        lpVtbl[2] = (delegate* unmanaged<XAudio2EngineCallback*, HRESULT, void>)&OnCriticalError;

        return lpVtbl;
    }

    /// <summary>
    /// The vtable pointer for the current instance.
    /// </summary>
    private void** lpVtbl;

    public static void Create(out XAudio2EngineCallback* callback)
    {
        XAudio2EngineCallback* @this = (XAudio2EngineCallback*)NativeMemory.Alloc((nuint)sizeof(XAudio2EngineCallback));
        @this->lpVtbl = Vtbl;

        callback = @this;
    }

    public static void Free(XAudio2EngineCallback* callback)
    {
        NativeMemory.Free(callback);
    }

    [UnmanagedCallersOnly]
    private static void OnProcessingPassStart(XAudio2EngineCallback* @this)
    {
    }

    [UnmanagedCallersOnly]
    private static void OnProcessingPassEnd(XAudio2EngineCallback* @this)
    {
    }

    [UnmanagedCallersOnly]
    private static void OnCriticalError(XAudio2EngineCallback* @this, HRESULT Error)
    {
    }
}
