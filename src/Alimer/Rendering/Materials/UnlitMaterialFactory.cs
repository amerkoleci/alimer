// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public sealed class UnlitMaterialFactory : GPUMaterialFactory<UnlitMaterial>
{
    public UnlitMaterialFactory(RenderSystem system)
        : base(system)
    {
        _bindGroupLayout = system.Device.CreateBindGroupLayout(
            "Unlit  Material BindGroupLayout",
            new BindGroupLayoutEntry(new BufferBindingLayout(BufferBindingType.Constant), 0, ShaderStages.Fragment),
            new BindGroupLayoutEntry(new TextureBindingLayout(), 0, ShaderStages.Fragment), // baseColorTexture
            new BindGroupLayoutEntry(SamplerDescriptor.Default, 0, ShaderStages.Fragment),  // baseColorSampler
                                                                                            // Static samplers
            new BindGroupLayoutEntry(SamplerDescriptor.PointClamp, 100, ShaderStages.All),          // SamplerPointClamp
            new BindGroupLayoutEntry(SamplerDescriptor.PointWrap, 101, ShaderStages.All),           // SamplerPointWrap
            new BindGroupLayoutEntry(SamplerDescriptor.PointMirror, 102, ShaderStages.All),         // SamplerPointMirror
            new BindGroupLayoutEntry(SamplerDescriptor.LinearClamp, 103, ShaderStages.All),         // SamplerLinearClamp
            new BindGroupLayoutEntry(SamplerDescriptor.LinearWrap, 104, ShaderStages.All),          // SamplerLinearWrap
            new BindGroupLayoutEntry(SamplerDescriptor.LinearMirror, 105, ShaderStages.All),        // SamplerLinearMirror
            new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicClamp, 106, ShaderStages.All),    // SamplerAnisotropicClamp
            new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicWrap, 107, ShaderStages.All),     // SamplerAnisotropicWrap
            new BindGroupLayoutEntry(SamplerDescriptor.AnisotropicMirror, 108, ShaderStages.All),   // SamplerAnisotropicMirror
            new BindGroupLayoutEntry(SamplerDescriptor.ComparisonDepth, 109, ShaderStages.All)      // SamplerAnisotropicMirror
            );
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _bindGroupLayout?.Dispose();
        }
    }

    protected override BindGroup CreateBindGroup(Material material) => throw new NotImplementedException();
    public override ShaderModule CreateFragmentShaderModule(Span<VertexBufferLayout> geometryLayout, Material material) => throw new NotImplementedException();
    protected override BindGroupLayout CreateBindGroupLayout() => throw new NotImplementedException();
}
