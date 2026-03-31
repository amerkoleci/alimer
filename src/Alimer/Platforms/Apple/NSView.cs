// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using static Alimer.Platforms.Apple.ObjectiveC;

namespace Alimer.Platforms.Apple;

internal readonly partial struct NSView
{
    #region Selectors
    private static ObjectiveCClass s_class => new("NSView"u8);

    private static Selector s_sel_wantsLayer => "wantsLayer"u8;
    private static Selector s_sel_setWantsLayer => "setWantsLayer:"u8;
    private static Selector s_sel_layer => "layer"u8;
    private static Selector s_sel_setLayer => "setLayer:"u8;
    #endregion 

    public nint Handle { get; }

    public NSView(nint handle) => Handle = handle;

    public static implicit operator NSView(nint handle) => new(handle);
    public static implicit operator nint(NSView value) => value.Handle;


    public bool wantsLayer
    {
        get => bool_objc_msgSend(Handle, s_sel_wantsLayer);
        set => objc_msgSend(Handle, s_sel_setWantsLayer, value);
    }

    public nint layer
    {
        get => IntPtr_objc_msgSend(Handle, s_sel_layer);
        set => objc_msgSend(Handle, s_sel_setLayer, value);
    }
}
