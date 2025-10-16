// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Text;

namespace Alimer.Platforms.Apple;

internal static unsafe partial class ObjectiveC
{
    public const string ObjCRuntime = "/usr/lib/libobjc.A.dylib";
    public const string Libdl = "libdl.dylib";
    public const string LibSystem = "/usr/lib/libSystem.dylib";

    public const string CoreGraphicsFramework = "/System/Library/Frameworks/CoreGraphics.framework/CoreGraphics";
    public const string AppKitFramework = "/System/Library/Frameworks/AppKit.framework/AppKit";
    public const string MetalFramework = "/System/Library/Frameworks/Metal.framework/Metal";
    public const string MetalKitFramework = "/System/Library/Frameworks/MetalKit.framework/MetalKit";

    [LibraryImport(ObjCRuntime, StringMarshalling = StringMarshalling.Utf8)]
    public static partial string sel_getName(nint selector);

    [LibraryImport(ObjCRuntime, StringMarshalling = StringMarshalling.Utf8)]
    public static unsafe partial nint sel_getUid(string name);

    [LibraryImport(ObjCRuntime, StringMarshalling = StringMarshalling.Utf8)]
    public static partial nint objc_getClass(string name);

    [LibraryImport(ObjCRuntime, StringMarshalling = StringMarshalling.Utf8)]
    public static partial nint class_getProperty(ObjectiveCClass @class, string name);

    [LibraryImport(ObjCRuntime, StringMarshalling = StringMarshalling.Utf8)]
    public static partial string class_getName(ObjectiveCClass @class);

    #region MessageSend
    [LibraryImport(ObjCRuntime, EntryPoint = "objc_msgSend")]
    public static partial void objc_msgSend(nint receiver, Selector selector);

    [LibraryImport(ObjCRuntime, EntryPoint = "objc_msgSend")]
    public static partial nint IntPtr_objc_msgSend(nint receiver, Selector selector);

    [LibraryImport(ObjCRuntime, EntryPoint = "objc_msgSend")]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool bool_objc_msgSend(nint receiver, Selector selector);

    [LibraryImport(ObjCRuntime, EntryPoint = "objc_msgSend")]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool bool_objc_msgSend(nint receiver, Selector selector, nuint value);

    [LibraryImport(ObjCRuntime, EntryPoint = "objc_msgSend")]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool bool_objc_msgSend(nint receiver, Selector selector, nint value);

    [LibraryImport(ObjCRuntime, EntryPoint = "objc_msgSend")]
    [return: MarshalAs(UnmanagedType.U1)]
    public static partial bool bool_objc_msgSend(nint receiver, Selector selector, ulong value);

    [LibraryImport(ObjCRuntime, EntryPoint = "objc_msgSend")]
    public static partial ulong ulong_objc_msgSend(nint receiver, Selector selector);
    #endregion

    public static void retain(nint receiver) => objc_msgSend(receiver, Selectors.Retain);
    public static void release(nint receiver) => objc_msgSend(receiver, Selectors.Release);
    public static ulong GetRetainCount(nint receiver) => ulong_objc_msgSend(receiver, Selectors.RetainCount);

    public static string GetUtf8String(byte* stringStart)
    {
        int characters = 0;
        while (stringStart[characters] != 0)
        {
            characters++;
        }

        return Encoding.UTF8.GetString(stringStart, characters);
    }
}
