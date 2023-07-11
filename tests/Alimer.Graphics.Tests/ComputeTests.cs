// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(Pipeline))]
public abstract class ComputeTests : GraphicsDeviceTestBase
{
    protected ComputeTests(GraphicsBackendType backendType)
        : base(backendType)
    {
    }

    [Test]
    public void ComputeTest1()
    {
        BindGroupLayoutDescription bindGroupLayoutDescription = new();
        using BindGroupLayout bindGroupLayout = GraphicsDevice.CreateBindGroupLayout(bindGroupLayoutDescription);
        Assert.IsNotNull(bindGroupLayout);

        PipelineLayoutDescription pipelineLayoutDescription = new(new[] { bindGroupLayout });
        using PipelineLayout pipelineLayout = GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription);
        Assert.IsNotNull(pipelineLayout);

        ShaderStageDescription computeShader = TestUtilities.CompileShader("ComputeTexture.hlsl", "computeMain", GraphicsDevice.Backend, ShaderStages.Compute);

        ComputePipelineDescription pipelineDescription = new ComputePipelineDescription(pipelineLayout, computeShader);
        using Pipeline pipeline = GraphicsDevice.CreateComputePipeline(pipelineDescription);
        Assert.IsNotNull(pipeline);
    }
}

[Category("D3D12")]
public class D3D12ComputeTests : ComputeTests
{
    public D3D12ComputeTests()
        : base(GraphicsBackendType.D3D12)
    {

    }
}

[Category("Vulkan")]
public class VulkanComputeTests : ComputeTests
{
    public VulkanComputeTests()
        : base(GraphicsBackendType.Vulkan)
    {

    }
}
