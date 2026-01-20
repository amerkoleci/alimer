// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using static Alimer.Platforms.Apple.ObjectiveC;

namespace Alimer.Platforms.Apple;

internal readonly partial struct NSWindow
{
    #region Selectors
    private static ObjectiveCClass s_class => new(nameof(NSWindow));

    private static readonly Selector s_sel_title = "title";
    private static readonly Selector s_sel_setTitle = "setTitle:";
    private static readonly Selector s_sel_contentView = "contentView";
    private static readonly Selector s_sel_setContentView = "setContentView:";
    #endregion 

    public nint Handle { get; }

    public NSWindow(nint handle) => Handle = handle;

    public static implicit operator NSWindow(nint handle) => new(handle);
    public static implicit operator nint(NSWindow value) => value.Handle;

    public NSString Title
    {
        get => new(IntPtr_objc_msgSend(Handle, s_sel_title));
        set => objc_msgSend(Handle, s_sel_setTitle, value.Handle);
    }

    public nint contentView
    {
        get => IntPtr_objc_msgSend(Handle, s_sel_contentView);
        set => objc_msgSend(Handle, s_sel_setContentView, value);
    }
}

internal readonly partial struct CAMetalLayer
{
    private static ObjectiveCClass s_class => new(nameof(CAMetalLayer));

    public nint Handle { get; }

    public CAMetalLayer(nint handle) => Handle = handle;

    public static implicit operator CAMetalLayer(nint handle) => new(handle);
    public static implicit operator nint(CAMetalLayer value) => value.Handle;

    public static CAMetalLayer New() => s_class.AllocInit<CAMetalLayer>();

    public static bool TryCast(nint layerPointer, out CAMetalLayer metalLayer)
    {
        NSObject layerObject = new(layerPointer);

        if (layerObject.IsKindOfClass(s_class))
        {
            metalLayer = new CAMetalLayer(layerPointer);
            return true;
        }

        metalLayer = default;
        return false;
    }
}
