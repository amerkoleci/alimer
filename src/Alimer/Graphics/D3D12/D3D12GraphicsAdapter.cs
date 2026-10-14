// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Utilities;
using Vortice.Win32;
using Vortice.Win32.Graphics;
using static Alimer.Utilities.MarshalUtilities;
using static Vortice.Win32.Apis;
using static Vortice.Win32.Graphics.DXGI;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12GraphicsAdapter : GraphicsAdapter, IDisposable
{
    private readonly ComPtr<IDXGIAdapter1> _handle;
    private readonly GraphicsMemoryHeapInfo[] _memoryHeaps = [];

    public D3D12GraphicsAdapter(
        D3D12GraphicsManager manager,
        ComPtr<IDXGIAdapter1> handle,
        ID3D12Device* device)
        : base(manager)
    {
        _handle = handle.Move();

        DXGI_ADAPTER_DESC1 adapterDesc;
        _handle.Get()->GetDesc1(&adapterDesc).ThrowIfFailed();

        // Init features
        Features = new D3D12Features(device);

        // Convert the adapter's D3D12 driver version to a readable string like "24.21.13.9793".
        string driverDescription = string.Empty;
        LARGE_INTEGER umdVersion;
        if (_handle.Get()->CheckInterfaceSupport(__uuidof<IDXGIDevice>(), &umdVersion) != DXGI_ERROR_UNSUPPORTED)
        {
            driverDescription = "D3D12 driver version ";

            long encodedVersion = umdVersion.QuadPart;
            for (int i = 0; i < 4; ++i)
            {
                ushort driverVersion = (ushort)((encodedVersion >> (48 - 16 * i)) & 0xFFFF);
                driverDescription += $"{driverVersion}.";
            }
        }

        DeviceName = MarshalUtilities.GetString(GetUtf16Span(in adapterDesc.Description[0], 128)) ?? string.Empty;
        VendorId = adapterDesc.VendorId;
        DeviceId = adapterDesc.DeviceId;

        // Detect adapter type.
        Type = GraphicsAdapterType.Other;
        if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0u)
        {
            Type = GraphicsAdapterType.Cpu;
        }
        else
        {
            Type = Features.UMA() ? GraphicsAdapterType.IntegratedGpu : GraphicsAdapterType.DiscreteGpu;
        }

        Debug.Assert(Constants.LUIDSize == sizeof(LUID));
        DeviceLUID = *(ulong*)&adapterDesc.AdapterLuid;

        _memoryHeaps = [];
        // https://microsoft.github.io/DirectX-Specs/d3d/D3D12GPUUploadHeaps.html describes the properties of D3D12 Default/Upload/Readback heaps.
        if (Features.UMA())
        {
            _memoryHeaps = new GraphicsMemoryHeapInfo[1];

            _memoryHeaps[0].Size = Math.Max(adapterDesc.DedicatedVideoMemory, adapterDesc.SharedSystemMemory);
            _memoryHeaps[0].Properties =
                GraphicsHeapProperty.DeviceLocal | GraphicsHeapProperty.HostVisible |
                GraphicsHeapProperty.HostUncached | GraphicsHeapProperty.HostCached;
        }
        else
        {
            _memoryHeaps = new GraphicsMemoryHeapInfo[2];
            _memoryHeaps[0].Size = adapterDesc.DedicatedVideoMemory;
            _memoryHeaps[0].Properties = GraphicsHeapProperty.DeviceLocal;

            _memoryHeaps[1].Size = adapterDesc.SharedSystemMemory;
            _memoryHeaps[1].Properties =
                GraphicsHeapProperty.HostVisible | GraphicsHeapProperty.HostCoherent |
                GraphicsHeapProperty.HostUncached | GraphicsHeapProperty.HostCached;
        }
    }

    public D3D12GraphicsManager DxManager => (D3D12GraphicsManager)base.Manager;

    public IDXGIAdapter1* Handle => _handle;
    public D3D12Features Features { get; }

    /// <inheritdoc />
    public override string DeviceName { get; }

    /// <inheritdoc />
    public override uint VendorId { get; }

    /// <inheritdoc />
    public override uint DeviceId { get; }

    /// <inheritdoc />
    public override GraphicsAdapterType Type { get; }
    /// <inheritdoc />
    public override ulong DeviceLUID { get; }
    /// <inheritdoc />
    public override ReadOnlySpan<GraphicsMemoryHeapInfo> MemoryHeaps => _memoryHeaps;

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new D3D12GraphicsDevice(this, description);

    public void Dispose()
    {
        _handle.Dispose();
    }
}
