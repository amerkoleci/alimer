// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using TerraFX.Interop.DirectX;
using static TerraFX.Interop.DirectX.D3D12_SRV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_UAV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_DSV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12_RTV_DIMENSION;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.DXGI_FORMAT;
using static TerraFX.Interop.DirectX.D3D12_DSV_FLAGS;
using DescriptorIndex = System.UInt32;
using System.Diagnostics;
using CommunityToolkit.Diagnostics;

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
                DSV = GetDSV(false, false);
            }
            else
            {
                RTVFormat = descriptor.Format.ToDxgiRTVFormat();
                RTV = GetRTV(RTVFormat);
            }
        }
    }

    public DXGI_FORMAT SRVFormat { get; }
    public DXGI_FORMAT RTVFormat { get; }
    public DXGI_FORMAT DSVFormat { get; }
    public D3D12_CPU_DESCRIPTOR_HANDLE RTV { get; }
    public D3D12_CPU_DESCRIPTOR_HANDLE DSV { get; }

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
            Shader4ComponentMapping = Swizzle.ToD3D12()
        };

        if (_texture.SampleCount > TextureSampleCount.Count1)
        {
            if (Dimension == TextureViewDimension.View2D)
            {
                viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
            }
            else
            {
                viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                viewDesc.Texture2DMSArray.FirstArraySlice = BaseArrayLayer;
                viewDesc.Texture2DMSArray.ArraySize = ArrayLayerCount;
            }
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

                case TextureViewDimension.View2DArray:
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

    public D3D12_CPU_DESCRIPTOR_HANDLE GetRTV(DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN)
    {
        int hash = HashCode.Combine(format);

        if (!_RTVs.TryGetValue(hash, out DescriptorIndex descriptorIndex))
        {
            D3D12_RENDER_TARGET_VIEW_DESC viewDesc = default;
            viewDesc.Format = format;

            if (Texture.IsMultisampled)
            {
                Debug.Assert(Texture.Dimension == TextureDimension.Texture2D);
                Debug.Assert(Texture.MipLevelCount == 1);
                Debug.Assert(ArrayLayerCount == 1);
                Debug.Assert(BaseArrayLayer == 0);
                Debug.Assert(MipLevelCount == 0);

                if (Dimension == TextureViewDimension.View2D)
                {
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = BaseArrayLayer;
                    viewDesc.Texture2DMSArray.ArraySize = ArrayLayerCount;
                }
            }
            else
            {
                switch (Dimension)
                {
                    case TextureViewDimension.View1D:
                    case TextureViewDimension.View1DArray:
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice = BaseMipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = BaseArrayLayer;
                        viewDesc.Texture1DArray.ArraySize = ArrayLayerCount;
                        break;

                    case TextureViewDimension.View2D:
                    case TextureViewDimension.View2DArray:
                    case TextureViewDimension.ViewCube:
                    case TextureViewDimension.ViewCubeArray:
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = BaseMipLevel;
                        viewDesc.Texture2DArray.FirstArraySlice = BaseArrayLayer;
                        viewDesc.Texture2DArray.ArraySize = ArrayLayerCount;
                        viewDesc.Texture2DArray.PlaneSlice = 0; // TODO
                        break;
                    case TextureViewDimension.View3D:
                        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                        viewDesc.Texture3D.MipSlice = BaseMipLevel;
                        viewDesc.Texture3D.FirstWSlice = BaseArrayLayer;
                        viewDesc.Texture3D.WSize = ArrayLayerCount;
                        break;
                }
            }

            descriptorIndex = _texture.DXDevice.RenderTargetViewHeap.AllocateDescriptors(1u);
            _RTVs.Add(hash, descriptorIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _texture.DXDevice.RenderTargetViewHeap.GetCpuHandle(descriptorIndex);
            _texture.DXDevice.Device->CreateRenderTargetView(_texture.Handle, &viewDesc, cpuHandle);
        }

        return _texture.DXDevice.RenderTargetViewHeap.GetCpuHandle(descriptorIndex);
    }

    public D3D12_CPU_DESCRIPTOR_HANDLE GetDSV(bool depthReadOnly, bool stencilReadOnly)
    {
        int hash = HashCode.Combine(DSVFormat, depthReadOnly, stencilReadOnly);

        if (!_DSVs.TryGetValue(hash, out DescriptorIndex descriptorIndex))
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = default;
            viewDesc.Format = DSVFormat;
            viewDesc.Flags = D3D12_DSV_FLAG_NONE;
            if (depthReadOnly && Format.IsDepthFormat())
            {
                viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
            }
            if (stencilReadOnly && Format.IsStencilFormat())
            {
                viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            }

            if (Texture.IsMultisampled)
            {
                Debug.Assert(Texture.Dimension == TextureDimension.Texture2D);
                Debug.Assert(Texture.MipLevelCount == 1);
                Debug.Assert(ArrayLayerCount == 1);
                Debug.Assert(BaseArrayLayer == 0);
                Debug.Assert(MipLevelCount == 0);

                if (Dimension == TextureViewDimension.View2D)
                {
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
                }
                else
                {
                    viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
                    viewDesc.Texture2DMSArray.FirstArraySlice = BaseArrayLayer;
                    viewDesc.Texture2DMSArray.ArraySize = ArrayLayerCount;
                }
            }
            else
            {
                switch (Dimension)
                {
                    case TextureViewDimension.View1D:
                    case TextureViewDimension.View1DArray:
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                        viewDesc.Texture1DArray.MipSlice = BaseMipLevel;
                        viewDesc.Texture1DArray.FirstArraySlice = BaseArrayLayer;
                        viewDesc.Texture1DArray.ArraySize = ArrayLayerCount;
                        break;

                    case TextureViewDimension.View2D:
                    case TextureViewDimension.View2DArray:
                    case TextureViewDimension.ViewCube:
                    case TextureViewDimension.ViewCubeArray:
                        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                        viewDesc.Texture2DArray.MipSlice = BaseMipLevel;
                        viewDesc.Texture2DArray.FirstArraySlice = BaseArrayLayer;
                        viewDesc.Texture2DArray.ArraySize = ArrayLayerCount;
                        break;
                    default:
                        ThrowHelper.ThrowInvalidOperationException();
                        break;
                }

                viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            }

            descriptorIndex = _texture.DXDevice.DepthStencilViewHeap.AllocateDescriptors(1u);
            _DSVs.Add(hash, descriptorIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = _texture.DXDevice.DepthStencilViewHeap.GetCpuHandle(descriptorIndex);
            _texture.DXDevice.Device->CreateDepthStencilView(_texture.Handle, &viewDesc, cpuHandle);
        }

        return _texture.DXDevice.DepthStencilViewHeap.GetCpuHandle(descriptorIndex);
    }
}
