// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Vortice.Win32;
using Vortice.Win32.Graphics;
using static Alimer.Utilities.UnsafeUtilities;
using static Vortice.Win32.Apis;
using static Vortice.Win32.Graphics.D3D;
using static Vortice.Win32.Graphics.D3D12;

namespace Alimer.Graphics.D3D12;

internal static unsafe partial class ID3D12Extensions
{
    public static HRESULT SetSDKVersion<TID3D12SDKConfiguration>(this ref TID3D12SDKConfiguration self, uint SDKVersion, string SDKPath)
        where TID3D12SDKConfiguration : unmanaged, ID3D12SDKConfiguration.Interface
    {
        fixed (byte* SDKPathPtr = SDKPath.GetUtf8Span())
        {
            return self.SetSDKVersion(SDKVersion, SDKPathPtr);
        }
    }

    public static HRESULT CreateDeviceFactory(this ref ID3D12SDKConfiguration1 self, uint SDKVersion, string SDKPath, Guid* riid, void** ppvFactory)
    {
        fixed (byte* SDKPathPtr = SDKPath.GetUtf8Span())
        {
            return self.CreateDeviceFactory(SDKVersion, SDKPathPtr, riid, ppvFactory);
        }
    }

    public static HRESULT GetConfigurationInterface<T>(this ref ID3D12DeviceFactory self, Guid classId, T** ppv)
        where T : unmanaged, INativeGuid
    {
        return self.GetConfigurationInterface(&classId, __uuidof<T>(), (void**)ppv);
    }

    public static void SetDxgiName<TIDXGIObject>(this ref TIDXGIObject self, ReadOnlySpan<char> name)
        where TIDXGIObject : unmanaged, IDXGIObject.Interface

    {
        if (name.IsEmpty)
        {
            _ = self.SetPrivateData(AsPointer(in WKPDID_D3DDebugObjectName), 0u, null);
        }
        else
        {
            fixed (char* pName = name)
            {
                _ = self.SetPrivateData(AsPointer(in WKPDID_D3DDebugObjectName), (uint)name.Length, (ushort*)pName);
            }
        }
    }

    public static void SetName<TID3D12Object>(ref this TID3D12Object self, Utf8ReadOnlyString name)
        where TID3D12Object : unmanaged, ID3D12Object.Interface
    {
        if (name.IsEmpty)
        {
            _ = self.SetName(null);
        }
        else
        {
            ReadOnlySpan<byte> label = (ReadOnlySpan<byte>)name;
            SetName(ref self, label); 
        }
    }

    public static void SetName<TID3D12Object>(ref this TID3D12Object self, ReadOnlySpan<byte> name)
        where TID3D12Object : unmanaged, ID3D12Object.Interface
    {
        string? nameString = name.GetString();

        if (string.IsNullOrEmpty(nameString))
        {
            _ = self.SetName(null);
        }
        else
        {
            fixed (char* namePtr = nameString)
            {
                _ = self.SetName(namePtr);
            }
        }
    }

    public static void SetName<TID3D12Object>(ref this TID3D12Object self, ReadOnlySpan<char> name)
        where TID3D12Object : unmanaged, ID3D12Object.Interface
    {
        if (name.IsEmpty)
        {
            _ = self.SetName(null);
        }
        else
        {
            fixed (char* namePtr = name)
            {
                _ = self.SetName(namePtr);
            }
        }
    }

    /// <summary>
    /// Creates a new <see cref="ID3D12Fence"/> for a given device.
    /// </summary>
    /// <param name="device">The target <see cref="ID3D12Device"/> to use to create the fence.</param>
    /// <returns>A pointer to the newly allocated <see cref="ID3D12Fence"/> instance.</returns>
    /// <exception cref="Exception">Thrown when the creation of the command queue fails.</exception>
    public static ComPtr<ID3D12Fence> CreateFence<TD3D12Device>(this ref TD3D12Device device)
        where TD3D12Device : unmanaged, ID3D12Device.Interface
    {
        using ComPtr<ID3D12Fence> d3D12Fence = default;

        device.CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            __uuidof<ID3D12Fence>(),
            (void**)d3D12Fence.GetAddressOf()
            ).ThrowIfFailed();

        return d3D12Fence.Move();
    }

    public static ComPtr<ID3D12CommandQueue> CreateCommandQueue<TD3D12Device>(this ref TD3D12Device device, D3D12_COMMAND_LIST_TYPE type)
        where TD3D12Device : unmanaged, ID3D12Device.Interface
    {
        using ComPtr<ID3D12CommandQueue> d3D12CommandQueue = default;

        D3D12_COMMAND_QUEUE_DESC d3D12CommandQueueDesc;
        d3D12CommandQueueDesc.Type = type;
        d3D12CommandQueueDesc.Priority = (int)D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        d3D12CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        d3D12CommandQueueDesc.NodeMask = 0;

        device.CreateCommandQueue(
            &d3D12CommandQueueDesc,
            __uuidof<ID3D12CommandQueue>(),
            (void**)d3D12CommandQueue.GetAddressOf()).ThrowIfFailed();

        return d3D12CommandQueue.Move();
    }

    public static ComPtr<ID3D12CommandSignature> CreateCommandSignature<TD3D12Device>(this ref TD3D12Device device, D3D12_COMMAND_SIGNATURE_DESC* desc)
        where TD3D12Device : unmanaged, ID3D12Device.Interface
    {
        using ComPtr<ID3D12CommandSignature> result = default;

        device.CreateCommandSignature(desc, null, __uuidof<ID3D12CommandSignature>(), (void**)result.GetAddressOf()).ThrowIfFailed();

        return result.Move();
    }
}
