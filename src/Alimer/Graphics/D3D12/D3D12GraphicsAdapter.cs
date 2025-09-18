// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.DirectX.D3D_FEATURE_LEVEL;
using static Alimer.Utilities.MarshalUtilities;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12GraphicsAdapter : GraphicsAdapter, IDisposable
{
    private readonly ComPtr<IDXGIAdapter1> _handle;
    private readonly ComPtr<ID3D12Device5> _device = default;

    public D3D12GraphicsAdapter(D3D12GraphicsManager manager, ComPtr<IDXGIAdapter1> handle)
        : base(manager)
    {
        _handle = handle.Move();

        DXGI_ADAPTER_DESC1 adapterDesc;
        ThrowIfFailed(_handle.Get()->GetDesc1(&adapterDesc));

        if (D3D12CreateDevice((IUnknown*)_handle.Get(),
            D3D_FEATURE_LEVEL_12_0,
            __uuidof<ID3D12Device5>(),
            _device.GetVoidAddressOf()).FAILED)
        {
            return;
        }

        // Init features
        Features = new D3D12Features((ID3D12Device*)_device.Get());

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

        DeviceName = GetUtf16Span(in adapterDesc.Description[0], 128).GetString() ?? string.Empty;

        // Detect adapter type.
        Type = GraphicsAdapterType.Other;
        if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0u)
        {
            Type = GraphicsAdapterType.Cpu;
        }
        else
        {
            //Type = _features.UMA() ? GpuAdapterType.IntegratedGpu : GpuAdapterType.DiscreteGpu;
            Type = GraphicsAdapterType.DiscreteGpu;
        }

        IsSuitable = true;
    }

    public IDXGIAdapter1* Handle => _handle;
    public ID3D12Device5* Device => _device;
    public bool IsSuitable { get; }
    public D3D12Features Features { get; }

    public override GraphicsAdapterProperties AdapterInfo => throw new NotImplementedException();

    public override GraphicsDeviceLimits Limits => throw new NotImplementedException();

    public override string DeviceName { get; }

    public override GraphicsAdapterType Type { get; }

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new D3D12GraphicsDevice(this, description);

    public void Dispose()
    {
        _device.Dispose();
        _handle.Dispose();
    }
}
