// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Numerics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Graphics;

public abstract class ComputeContext : CopyContext
{
    protected bool _insideRenderPass;

    protected ComputeContext()
    {
    }

    /// <inheritdoc />
    public override QueueType QueueType => QueueType.Compute; 

    protected virtual void Reset(uint frameIndex)
    {
        _hasLabel = false;
        _insideRenderPass = false;
        //frameAllocators[frameIndex].Reset();
    }

    public abstract void SetPipeline(Pipeline pipeline);

    public void Dispatch1D(uint threadCountX, uint groupSizeX = 64u)
    {
        Dispatch(
            MathHelper.DivideByMultiple(threadCountX, groupSizeX),
            1u,
            1u);
    }

    public void Dispatch2D(uint threadCountX, uint threadCountY, uint groupSizeX = 8u, uint groupSizeY = 8u)
    {
        Dispatch(
            MathHelper.DivideByMultiple(threadCountX, groupSizeX),
            MathHelper.DivideByMultiple(threadCountY, groupSizeX),
            1u
        );
    }

    public void Dispatch3D(uint threadCountX, uint threadCountY, uint threadCountZ, uint groupSizeX, uint groupSizeY, uint groupSizeZ)
    {
        Dispatch(
            MathHelper.DivideByMultiple(threadCountX, groupSizeX),
            MathHelper.DivideByMultiple(threadCountY, groupSizeY),
            MathHelper.DivideByMultiple(threadCountZ, groupSizeZ)
        );
    }

    public void Dispatch(uint groupCountX, uint groupCountY, uint groupCountZ)
    {
        Guard.IsFalse(_insideRenderPass);

        DispatchCore(groupCountX, groupCountY, groupCountZ);
    }

    protected abstract void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ);
}
