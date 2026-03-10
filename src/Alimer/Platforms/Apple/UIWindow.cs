// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using static Alimer.Platforms.Apple.ObjectiveC;

namespace Alimer.Platforms.Apple;

internal readonly partial struct CALayer
{
    #region Selectors
    private static Selector s_sel_addSublayer => "addSublayer:";
    #endregion 

    public nint Handle { get; }

    public CALayer(nint handle) => Handle = handle;

    public static implicit operator CALayer(nint handle) => new(handle);
    public static implicit operator nint(CALayer value) => value.Handle;

    public void addSublayer(nint layer)
    {
        objc_msgSend(Handle, s_sel_addSublayer, layer);
    }
}

internal readonly partial struct UIView
{
    #region Selectors
    private static ObjectiveCClass s_class => new(nameof(UIView));

    private static Selector s_sel_layer => "layer";
    private static Selector s_sel_setLayer => "setLayer:";
    private static readonly Selector s_sel_frame = "frame";
    #endregion 

    public nint Handle { get; }

    public UIView(nint handle) => Handle = handle;

    public static implicit operator UIView(nint handle) => new(handle);
    public static implicit operator nint(UIView value) => value.Handle;

    public CALayer layer
    {
        get => objc_msgSend<CALayer>(Handle, s_sel_layer);
        set => objc_msgSend(Handle, s_sel_setLayer, value.Handle);
    }

    public CGRect frame => CGRect_objc_msgSend(Handle, s_sel_frame);
}

internal readonly partial struct UIViewController
{
    #region Selectors
    private static ObjectiveCClass s_class => new(nameof(UIViewController));

    private static readonly Selector s_sel_view = "view";
    #endregion 

    public nint Handle { get; }

    public UIViewController(nint handle) => Handle = handle;

    public static implicit operator UIViewController(nint handle) => new(handle);
    public static implicit operator nint(UIViewController value) => value.Handle;

    public UIView View
    {
        get => IntPtr_objc_msgSend(Handle, s_sel_view);
    }
}


internal readonly partial struct UIWindow
{
    #region Selectors
    private static ObjectiveCClass s_class => new(nameof(UIWindow));

    private static readonly Selector s_sel_rootViewController = "rootViewController";
    private static readonly Selector s_sel_setRootViewController = "setRootViewController:";
    private static readonly Selector s_makeKeyAndVisible = "makeKeyAndVisible";
    #endregion 

    public nint Handle { get; }

    public UIWindow(nint handle) => Handle = handle;

    public static implicit operator UIWindow(nint handle) => new(handle);
    public static implicit operator nint(UIWindow value) => value.Handle;

    public UIViewController RootViewController
    {
        get => IntPtr_objc_msgSend(Handle, s_sel_rootViewController);
        set => objc_msgSend(Handle, s_sel_setRootViewController, value.Handle);
    }

    public void MakeKeyAndVisible()
    {
        objc_msgSend(Handle, s_makeKeyAndVisible);
    }
}
