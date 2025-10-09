// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using CommunityToolkit.Diagnostics;
using static Alimer.Graphics.Constants;

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

    public void SetPipeline(Pipeline pipeline)
    {
        Guard.IsNotNull(pipeline, nameof(pipeline));

        SetPipelineCore(pipeline);
    }

    public void SetBindGroup(uint groupIndex, BindGroup bindGroup)
    {
        Guard.IsNotNull(bindGroup, nameof(bindGroup));
        Guard.IsTrue(groupIndex < MaxBindGroups, nameof(groupIndex));

        SetBindGroupCore(groupIndex, bindGroup);
    }

    public unsafe void SetPushConstants<T>(uint pushConstantIndex, T data)
         where T : unmanaged
    {
        SetPushConstantsCore(pushConstantIndex, &data, (uint)sizeof(T));
    }

    public unsafe void SetPushConstants(uint pushConstantIndex, void* data, uint size)
    {
        SetPushConstantsCore(pushConstantIndex, data, size);
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
        PreDispatchValidation();

        DispatchCore(groupCountX, groupCountY, groupCountZ);
    }

    public void DispatchIndirect(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset = 0)
    {
        PreDispatchValidation();
        ValidateIndirectBuffer(indirectBuffer);
        ValidateIndirectOffset(indirectBufferOffset);

        DispatchIndirectCore(indirectBuffer, indirectBufferOffset);
    }

    protected abstract void SetPipelineCore(Pipeline pipeline);
    protected abstract void SetBindGroupCore(uint groupIndex, BindGroup bindGroup);
    protected unsafe abstract void SetPushConstantsCore(uint pushConstantIndex, void* data, uint size);
    protected abstract void DispatchCore(uint groupCountX, uint groupCountY, uint groupCountZ);
    protected abstract void DispatchIndirectCore(GraphicsBuffer indirectBuffer, ulong indirectBufferOffset);

    #region Validation
    [Conditional("VALIDATE_USAGE")]
    private void PreDispatchValidation()
    {
        if (_insideRenderPass)
        {
            throw new GraphicsException($"Dispatch needs to happen Outside render pass.");
        }
    }

    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateIndirectBuffer(GraphicsBuffer indirectBuffer)
    {
        if ((indirectBuffer.Usage & BufferUsage.Indirect) == 0)
        {
            throw new GraphicsException($"{nameof(indirectBuffer)} parameter must have been created with BufferUsage.Indirect.");
        }
    }

    [Conditional("VALIDATE_USAGE")]
    protected static void ValidateIndirectOffset(ulong offset)
    {
        if ((offset % 4) != 0)
        {
            throw new GraphicsException($"{nameof(offset)} must be a multiple of 4.");
        }
    }
    #endregion Validation
}
