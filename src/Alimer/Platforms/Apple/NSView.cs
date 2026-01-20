// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using static Alimer.Platforms.Apple.ObjectiveC;

namespace Alimer.Platforms.Apple;

internal readonly partial struct NSView
{
    #region Selectors
    private static ObjectiveCClass s_class => new(nameof(NSView));

    private static Selector s_sel_wantsLayer => "wantsLayer";
    private static Selector s_sel_setWantsLayer => "setWantsLayer:";
    private static Selector s_sel_layer => "layer";
    private static Selector s_sel_setLayer => "setLayer:";
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
