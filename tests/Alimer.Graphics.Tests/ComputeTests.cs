// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Xunit;

namespace Alimer.Graphics.Tests;

public abstract class ComputeTests : GraphicsDeviceTestBase
{
    protected ComputeTests(GraphicsBackendType backendType)
        : base(backendType)
    {
    }

    [Fact]
    public void ComputeTest1()
    {
        BindGroupLayoutDescription bindGroupLayoutDescription = new();
        using BindGroupLayout bindGroupLayout = GraphicsDevice.CreateBindGroupLayout(bindGroupLayoutDescription);
        Assert.NotNull(bindGroupLayout);

        PipelineLayoutDescription pipelineLayoutDescription = new(new[] { bindGroupLayout });
        using PipelineLayout pipelineLayout = GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription);
        Assert.NotNull(pipelineLayout);

        ShaderStageDescription computeShader = TestUtilities.CompileShader("ComputeTexture.hlsl", "computeMain", GraphicsDevice.Backend, ShaderStage.Compute);

        ComputePipelineDescription pipelineDescription = new ComputePipelineDescription(pipelineLayout, computeShader);
        using Pipeline pipeline = GraphicsDevice.CreateComputePipeline(pipelineDescription);
        Assert.NotNull(pipeline);
    }
}

[Trait("Backend", "D3D12")]
public class D3D12ComputeTests : ComputeTests
{
    public D3D12ComputeTests()
        : base(GraphicsBackendType.D3D12)
    {

    }
}

[Trait("Backend", "Vulkan")]
public class VulkanComputeTests : ComputeTests
{
    public VulkanComputeTests()
        : base(GraphicsBackendType.Vulkan)
    {

    }
}
