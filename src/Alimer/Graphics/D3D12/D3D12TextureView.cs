// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_SRV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_UAV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_DSV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_RTV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using DescriptorIndex = System.UInt32;
using System.Diagnostics;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12TextureView : TextureView
{
    private readonly D3D12Texture _texture;
    private readonly Dictionary<int, DescriptorIndex> _RTVs = [];
    private readonly Dictionary<int, DescriptorIndex> _DSVs = [];

    public D3D12TextureView(D3D12Texture texture, in TextureViewDescriptor descriptor)
        : base(texture, in descriptor)
    {
        _texture = texture;

        // TODO: Take look at D3DShaderResourceViewFormat
        if (texture.Usage.HasFlag(TextureUsage.ShaderRead))
        {
            SRVFormat = descriptor.Format.ToDxgiSRVFormat();
        }

        if (texture.Usage.HasFlag(TextureUsage.RenderTarget))
        {
            bool isDepthStencil = descriptor.Format.IsDepthStencilFormat();
            if (isDepthStencil)
            {
                DSVFormat = descriptor.Format.ToDxgiDSVFormat();
                //DSV = GetDSV(desc, textureView->DSVFormat, false);
                //DSVReadOnly = GetDSV(desc, textureView->DSVFormat, true);
            }
            else
            {
                RTVFormat = descriptor.Format.ToDxgiRTVFormat();
                //RTV = GetRTV(desc, textureView->RTVFormat);
            }
        }
    }

    public DXGI_FORMAT SRVFormat { get; }
    public DXGI_FORMAT RTVFormat { get; }
    public DXGI_FORMAT DSVFormat { get; }
    public D3D12_CPU_DESCRIPTOR_HANDLE RTV { get; }
    public D3D12_CPU_DESCRIPTOR_HANDLE DSV { get; }
    public D3D12_CPU_DESCRIPTOR_HANDLE DSVReadOnly { get; }

    /// <inheitdoc />
    internal override void Destroy()
    {
        foreach (DescriptorIndex index in _RTVs.Values)
        {
            _texture.DXDevice.RenderTargetViewHeap.ReleaseDescriptors(index);
        }
        _RTVs.Clear();

        foreach (DescriptorIndex index in _DSVs.Values)
        {
            _texture.DXDevice.DepthStencilViewHeap.ReleaseDescriptors(index);
        }
        _DSVs.Clear();
    }

    public D3D12_SHADER_RESOURCE_VIEW_DESC GetSRV()
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = new()
        {
            Format = SRVFormat,
            Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
        };

        if (_texture.SampleCount > TextureSampleCount.Count1)
        {
            viewDesc.ViewDimension = (Dimension == TextureViewDimension.Texture2DArray) ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            switch (Dimension)
            {
                case TextureViewDimension.View1D:
                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                    viewDesc.Texture1D.MostDetailedMip = BaseMipLevel;
                    viewDesc.Texture1D.MipLevels = MipLevelCount;
                    viewDesc.Texture1D.ResourceMinLODClamp = 0;
                    break;

                case TextureViewDimension.View2D:
                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                    viewDesc.Texture2D.MostDetailedMip = BaseMipLevel;
                    viewDesc.Texture2D.MipLevels = MipLevelCount;
                    viewDesc.Texture2D.PlaneSlice = 0; // TODO
                    viewDesc.Texture2D.ResourceMinLODClamp = 0;
                    break;

                case TextureViewDimension.View3D:
                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                    viewDesc.Texture3D.MostDetailedMip = BaseMipLevel;
                    viewDesc.Texture3D.MipLevels = MipLevelCount;
                    viewDesc.Texture3D.ResourceMinLODClamp = 0;
                    break;

                case TextureViewDimension.ViewCube:
                    Debug.Assert(_texture.Dimension == TextureDimension.Texture2D);
                    Debug.Assert(ArrayLayerCount % 6 == 0);

                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    viewDesc.TextureCube.MostDetailedMip = BaseMipLevel;
                    viewDesc.TextureCube.MipLevels = MipLevelCount;
                    viewDesc.TextureCube.ResourceMinLODClamp = 0;
                    break;

                case TextureViewDimension.View1DArray:
                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    viewDesc.Texture1DArray.MostDetailedMip = BaseMipLevel;
                    viewDesc.Texture1DArray.MipLevels = MipLevelCount;
                    viewDesc.Texture1DArray.FirstArraySlice = BaseArrayLayer;
                    viewDesc.Texture1DArray.ArraySize = ArrayLayerCount;
                    viewDesc.Texture1DArray.ResourceMinLODClamp = 0;
                    break;

                case TextureViewDimension.Texture2DArray:
                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    viewDesc.Texture2DArray.MostDetailedMip = BaseMipLevel;
                    viewDesc.Texture2DArray.MipLevels = MipLevelCount;
                    viewDesc.Texture2DArray.FirstArraySlice = BaseArrayLayer;
                    viewDesc.Texture2DArray.ArraySize = ArrayLayerCount;
                    viewDesc.Texture2DArray.PlaneSlice = 0; // TODO
                    viewDesc.Texture2DArray.ResourceMinLODClamp = 0;
                    break;

                case TextureViewDimension.ViewCubeArray:
                    Debug.Assert(_texture.Dimension == TextureDimension.Texture2D);
                    Debug.Assert(ArrayLayerCount % 6 == 0);

                    viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    viewDesc.TextureCubeArray.MostDetailedMip = BaseMipLevel;
                    viewDesc.TextureCubeArray.MipLevels = MipLevelCount;
                    viewDesc.TextureCubeArray.First2DArrayFace = BaseArrayLayer;
                    viewDesc.TextureCubeArray.NumCubes = ArrayLayerCount / 6;
                    viewDesc.TextureCubeArray.ResourceMinLODClamp = 0;
                    break;

                default:
                    throw new InvalidOperationException("Invalid texture view dimension for SRV.");
            }
        }

        return viewDesc;
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(uint mipSlice, uint arraySlice, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
    {
        int hash = HashCode.Combine(mipSlice, arraySlice, format);

        if (!_RTVs.TryGetValue(hash, out DescriptorIndex descriptorIndex))
        {
            D3D12_RESOURCE_DESC desc = _texture.Handle->GetDesc();

            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = default;
            viewDesc.Format = format;
            viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

            descriptorIndex = _texture.DXDevice.RenderTargetViewHeap.AllocateDescriptors(1u);
            _RTVs.Add(hash, descriptorIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _texture.DXDevice.RenderTargetViewHeap.GetCpuHandle(descriptorIndex);
            _texture.DXDevice.Device->CreateRenderTargetView(_texture.Handle, &viewDesc, cpuHandle);
        }

        return _texture.DXDevice.RenderTargetViewHeap.GetCpuHandle(descriptorIndex);
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(uint mipSlice, uint arraySlice, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
    {
        int hash = HashCode.Combine(mipSlice, arraySlice, format);

        if (!_DSVs.TryGetValue(hash, out DescriptorIndex descriptorIndex))
        {
            D3D12_RESOURCE_DESC desc = _texture.Handle->GetDesc();

            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = default;
            viewDesc.Format = format;
            viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

            descriptorIndex = _texture.DXDevice.DepthStencilViewHeap.AllocateDescriptors(1u);
            _DSVs.Add(hash, descriptorIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _texture.DXDevice.DepthStencilViewHeap.GetCpuHandle(descriptorIndex);
            _texture.DXDevice.Device->CreateDepthStencilView(_texture.Handle, &viewDesc, cpuHandle);
        }

        return _texture.DXDevice.DepthStencilViewHeap.GetCpuHandle(descriptorIndex);
    }
}
