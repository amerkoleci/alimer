// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;

namespace Alimer.Rendering;

public sealed class UnlitMaterialFactory : GPUMaterialFactory<UnlitMaterial>
{
    public UnlitMaterialFactory(RenderSystem system)
        : base(system)
    {
    }

    /// <summary>Finalizes an instance of the <see cref="UnlitMaterialFactory" /> class.</summary>
    ~UnlitMaterialFactory() => Dispose(disposing: false);


    protected override int Write(UnlitMaterial material, out int materialIndex) => throw new NotImplementedException();

    protected override PipelineLayout CreatePipelineLayout(bool skinned) => throw new NotImplementedException();
    public override ShaderModule CreateFragmentShaderModule(Span<VertexBufferLayout> geometryLayout, Material material) => throw new NotImplementedException();
}
