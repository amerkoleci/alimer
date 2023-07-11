// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture]
public class ComputeTests : GraphicsDeviceTestBase
{
    protected ComputeTests(GraphicsBackendType backendType)
        : base(backendType)
    {
    }

    [TestCase]
    public void ComputeTest1()
    {
        PipelineLayoutDescription pipelineLayoutDescription = new();
        using PipelineLayout pipelineLayout = GraphicsDevice.CreatePipelineLayout(pipelineLayoutDescription);
        Assert.IsNotNull(pipelineLayout);

        ShaderStageDescription computeShader = TestUtilities.CompileShader("ComputeTexture.hlsl", "computeMain", GraphicsDevice.Backend, ShaderStages.Compute);

        ComputePipelineDescription pipelineDescription = new ComputePipelineDescription(pipelineLayout, computeShader);
        using Pipeline pipeline = GraphicsDevice.CreateComputePipeline(pipelineDescription);
        Assert.IsNotNull(pipeline);
    }
}

[TestFixture(Description = "D3D12ComputeTests", TestOf = typeof(Pipeline))]
public class D3D12ComputeTests : ComputeTests
{
    public D3D12ComputeTests()
        : base(GraphicsBackendType.D3D12)
    {

    }
}

[TestFixture(Description = "VulkanComputeTests", TestOf = typeof(Pipeline))]
public class VulkanComputeTests : ComputeTests
{
    public VulkanComputeTests()
        : base(GraphicsBackendType.Vulkan)
    {

    }
}
