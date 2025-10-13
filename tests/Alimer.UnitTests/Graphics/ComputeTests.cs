// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using NUnit.Framework;

namespace Alimer.Graphics.Tests;

[TestFixture(TestOf = typeof(GraphicsDevice))]
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
        using BindGroupLayout bindGroupLayout = Device.CreateBindGroupLayout(bindGroupLayoutDescription);
        Assert.That(() => bindGroupLayout, Is.Not.Null);

        PipelineLayoutDescription pipelineLayoutDescription = new([bindGroupLayout]);
        using PipelineLayout pipelineLayout = Device.CreatePipelineLayout(pipelineLayoutDescription);
        Assert.That(() => pipelineLayout, Is.Not.Null);

        ShaderStageDescription computeShader = TestUtilities.CompileShader("ComputeTexture.hlsl", "computeMain", Manager.BackendType, ShaderStages.Compute);

        ComputePipelineDescription pipelineDescription = new(pipelineLayout, computeShader);
        using Pipeline pipeline = Device.CreateComputePipeline(pipelineDescription);
        Assert.That(() => pipeline, Is.Not.Null);
    }
}

[TestFixture(TestOf = typeof(GraphicsDevice))]
public class D3D12ComputeTests : ComputeTests
{
    public D3D12ComputeTests()
        : base(GraphicsBackendType.D3D12)
    {

    }
}

[TestFixture(TestOf = typeof(GraphicsDevice))]
public class VulkanComputeTests : ComputeTests
{
    public VulkanComputeTests()
        : base(GraphicsBackendType.Vulkan)
    {

    }
}
