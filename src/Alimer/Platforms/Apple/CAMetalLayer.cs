// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NSUInteger = System.UInt64;
using static Alimer.Platforms.Apple.ObjectiveC;

namespace Alimer.Platforms.Apple;

internal readonly partial struct CAMetalDrawable
{
    #region Selectors
    private static Selector s_sel_layer => "layer";
    private static Selector s_sel_setLayer => "setLayer:";
    private static readonly Selector s_sel_frame = "frame";
    #endregion 

    public nint Handle { get; }

    public CAMetalDrawable(nint handle) => Handle = handle;

    public static implicit operator CAMetalDrawable(nint handle) => new(handle);
    public static implicit operator nint(CAMetalDrawable value) => value.Handle;

    //public MTLTexture texture => objc_msgSend<MTLTexture>(NativePtr, Selectors.texture);
}

internal readonly partial struct CAMetalLayer
{
    #region Selectors
    private static ObjectiveCClass s_class => new(nameof(CAMetalLayer));

    private static readonly Selector s_sel_maximumDrawableCount = "maximumDrawableCount";
    private static readonly Selector s_sel_setMaximumDrawableCount = "setMaximumDrawableCount:";

    private static readonly Selector s_sel_framebufferOnly = "framebufferOnly";
    private static readonly Selector s_sel_setFramebufferOnly = "setFramebufferOnly:";

    private static readonly Selector s_sel_isOpaque = "isOpaque";
    private static readonly Selector s_sel_setOpaque = "setOpaque:";

    private static readonly Selector s_sel_displaySyncEnabled = "displaySyncEnabled";
    private static readonly Selector s_sel_setDisplaySyncEnabled = "setDisplaySyncEnabled:";

    private static readonly Selector s_sel_drawableSize = "drawableSize";
    private static readonly Selector s_sel_setDrawableSize = "setDrawableSize:";


    private static readonly Selector s_sel_frame = "frame";
    private static readonly Selector s_sel_setFrame = "setFrame:";

    private static readonly Selector s_sel_nextDrawable = "nextDrawable";
    #endregion

    public nint Handle { get; }

    public CAMetalLayer(nint handle) => Handle = handle;

    public static implicit operator CAMetalLayer(nint handle) => new(handle);
    public static implicit operator nint(CAMetalLayer value) => value.Handle;

    public NSUInteger maximumDrawableCount
    {
        get => ulong_objc_msgSend(Handle, s_sel_maximumDrawableCount);
        set => objc_msgSend(Handle, s_sel_setMaximumDrawableCount, value);
    }

    public bool framebufferOnly
    {
        get => bool_objc_msgSend(Handle, s_sel_framebufferOnly);
        set => objc_msgSend(Handle, s_sel_setFramebufferOnly, value);
    }

    public bool opaque
    {
        get => bool_objc_msgSend(Handle, s_sel_isOpaque);
        set => objc_msgSend(Handle, s_sel_setOpaque, value);
    }

    public bool displaySyncEnabled
    {
        get => bool_objc_msgSend(Handle, s_sel_displaySyncEnabled);
        set => objc_msgSend(Handle, s_sel_setDisplaySyncEnabled, value);
    }

    public CGSize drawableSize
    {
        get => CGSize_objc_msgSend(Handle, s_sel_drawableSize);
        set => objc_msgSend(Handle, s_sel_setDrawableSize, value);
    }

    public CGRect frame
    {
        get => CGRect_objc_msgSend(Handle, s_sel_frame);
        set => objc_msgSend(Handle, s_sel_setFrame, value);
    }

    public CAMetalDrawable nextDrawable() => objc_msgSend<CAMetalDrawable>(Handle, s_sel_nextDrawable);

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
