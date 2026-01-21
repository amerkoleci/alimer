// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Utilities.MarshalUtilities;
using static TerraFX.Interop.DirectX.DXGI;
using static TerraFX.Interop.DirectX.DXGI_ADAPTER_FLAG;
using static TerraFX.Interop.Windows.Windows;

namespace Alimer.Graphics.D3D12;

internal sealed unsafe class D3D12GraphicsAdapter : GraphicsAdapter, IDisposable
{
    private readonly ComPtr<IDXGIAdapter1> _handle;

    public D3D12GraphicsAdapter(
        D3D12GraphicsManager manager,
        ComPtr<IDXGIAdapter1> handle,
        ID3D12Device* device)
        : base(manager)
    {
        _handle = handle.Move();

        DXGI_ADAPTER_DESC1 adapterDesc;
        ThrowIfFailed(_handle.Get()->GetDesc1(&adapterDesc));

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

        DeviceName = GetUtf16Span(in adapterDesc.Description[0], 128).GetString() ?? string.Empty;
        VendorId = adapterDesc.VendorId;
        DeviceId = adapterDesc.DeviceId;

        // Detect adapter type.
        Type = GraphicsAdapterType.Other;
        if ((adapterDesc.Flags & (uint)DXGI_ADAPTER_FLAG_SOFTWARE) != 0u)
        {
            Type = GraphicsAdapterType.Cpu;
        }
        else
        {
            Type = Features.UMA() ? GraphicsAdapterType.IntegratedGpu : GraphicsAdapterType.DiscreteGpu;
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

    protected override GraphicsDevice CreateDeviceCore(in GraphicsDeviceDescription description) => new D3D12GraphicsDevice(this, description);

    public void Dispose()
    {
        _handle.Dispose();
    }
}
