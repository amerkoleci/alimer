// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using WebGPU;
using static WebGPU.WebGPU;
using static Alimer.Graphics.Constants;
using System.Diagnostics;

namespace Alimer.Graphics.WebGPU;

internal unsafe class WebGPUPipeline : Pipeline
{
    private readonly WebGPUGraphicsDevice _device;
    private readonly WebGPUPipelineLayout _layout;

    public WebGPUPipeline(WebGPUGraphicsDevice device, in RenderPipelineDescription description)
        : base(PipelineType.Render, description.Label)
    {
        _device = device;
        _layout = (WebGPUPipelineLayout)description.Layout;
    }

    public WebGPUPipeline(WebGPUGraphicsDevice device, in ComputePipelineDescription description)
        : base(PipelineType.Compute, description.Label)
    {
        _device = device;
        _layout = (WebGPUPipelineLayout)description.Layout;
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _device;

    /// <inheritdoc />
    public override PipelineLayout Layout => _layout;

    public WGPURenderPipeline Handle { get; }

    /// <summary>
    /// Finalizes an instance of the <see cref="WebGPUPipeline" /> class.
    /// </summary>
    ~WebGPUPipeline() => Dispose(disposing: false);

    /// <inheritdoc />
    protected override void OnLabelChanged(string newLabel)
    {
    }

    /// <inheitdoc />
    protected internal override void Destroy()
    {
    }
}
