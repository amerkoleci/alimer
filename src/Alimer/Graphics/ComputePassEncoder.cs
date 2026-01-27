// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract unsafe class ComputePassEncoder : CommandEncoder
{
    protected ComputePassEncoder()
    {
    }

    protected virtual void Reset()
    {
        _hasLabel = false;
    }

    public void SetPipeline(ComputePipeline pipeline)
    {
        Guard.IsNotNull(pipeline, nameof(pipeline));

        SetPipelineCore(pipeline);
    }

    public void Dispatch1D(uint threadCountX, uint groupSizeX = 64u)
    {
        Dispatch(
            MathUtilities.DivideByMultiple(threadCountX, groupSizeX),
            1u,
            1u);
    }

    public void Dispatch2D(uint threadCountX, uint threadCountY, uint groupSizeX = 8u, uint groupSizeY = 8u)
    {
        Dispatch(
            MathUtilities.DivideByMultiple(threadCountX, groupSizeX),
            MathUtilities.DivideByMultiple(threadCountY, groupSizeX),
            1u
        );
    }

    public void Dispatch3D(uint threadCountX, uint threadCountY, uint threadCountZ, uint groupSizeX, uint groupSizeY, uint groupSizeZ)
    {
        Dispatch(
            MathUtilities.DivideByMultiple(threadCountX, groupSizeX),
            MathUtilities.DivideByMultiple(threadCountY, groupSizeY),
            MathUtilities.DivideByMultiple(threadCountZ, groupSizeZ)
        );
    }

    public void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        DispatchCore(groupCountX, groupCountY, groupCountZ);
    }

    public void DispatchIndirect(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset = 0)
    {
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);

        DispatchIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    protected abstract void SetPipelineCore(ComputePipeline pipeline);
    protected abstract void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ);
    protected abstract void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);
}
