// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.Constants;
using static TerraFX.Interop.Windows.Windows;
using static TerraFX.Interop.DirectX.DirectX;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BARRIER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BARRIER_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE;
using static TerraFX.Interop.DirectX.D3D12_SHADING_RATE_COMBINER;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RENDER_PASS_ENDING_ACCESS_TYPE;
using static Alimer.Graphics.D3D12.D3D12Utils;
using Alimer.Graphics.D3D;
using System.Diagnostics;
using Alimer.Numerics;
using Alimer.Utilities;

namespace Alimer.Graphics.D3D12;

internal unsafe class D3D12CommandBuffer : CommandBuffer
{
    private const int MaxBarriers = 16;

    /// <summary>
    /// Allowed states for <see cref="CommandQueueType.Compute"/>
    /// </summary>
    private static readonly D3D12_RESOURCE_STATES s_ValidComputeResourceStates =
       D3D12_RESOURCE_STATE_UNORDERED_ACCESS
       | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
       | D3D12_RESOURCE_STATE_COPY_DEST
       | D3D12_RESOURCE_STATE_COPY_SOURCE;

    /// <summary>
    /// Allowed states for <see cref="CommandQueueType.Copy"/>
    /// </summary>
    private static readonly D3D12_RESOURCE_STATES s_ValidCopyResourceStates = D3D12_RESOURCE_STATE_COPY_DEST | D3D12_RESOURCE_STATE_COPY_SOURCE;

    private readonly D3D12CommandQueue _queue;
    private readonly ComPtr<ID3D12CommandAllocator>[] _commandAllocators;
    private readonly ComPtr<ID3D12GraphicsCommandList6> _commandList;

    private readonly D3D12RenderPassEncoder _renderPassEncoder;
    private readonly D3D12ComputePassEncoder _computePassEncoder;

    private readonly D3D12_RESOURCE_BARRIER[] _resourceBarriers = new D3D12_RESOURCE_BARRIER[MaxBarriers];
    private uint _numBarriersToFlush;

    private D3D12PipelineLayout? _currentPipelineLayout;

    private bool _bindGroupsDirty;
    private int _numBoundBindGroups;
    private readonly D3D12BindGroup[] _boundBindGroups = new D3D12BindGroup[MaxBindGroups];

    public D3D12CommandBuffer(D3D12CommandQueue queue)
    {
        _queue = queue;

        _commandAllocators = new ComPtr<ID3D12CommandAllocator>[queue.D3DDevice.MaxFramesInFlight];
        for (int i = 0; i < _commandAllocators.Length; ++i)
        {
            ThrowIfFailed(
                queue.D3DDevice.Device->CreateCommandAllocator(queue.CommandListType,
                __uuidof<ID3D12CommandAllocator>(), (void**)_commandAllocators[i].GetAddressOf())
            );
        }

        ThrowIfFailed(
           queue.D3DDevice.Device->CreateCommandList1(0, queue.CommandListType, D3D12_COMMAND_LIST_FLAG_NONE,
            __uuidof<ID3D12GraphicsCommandList6>(), (void**)_commandList.GetAddressOf()
        ));

        _renderPassEncoder = new D3D12RenderPassEncoder(this);
        _computePassEncoder = new D3D12ComputePassEncoder(this);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _queue.Device;

    public D3D12GraphicsDevice D3DDevice => _queue.D3DDevice;
    public ID3D12GraphicsCommandList6* CommandList => _commandList;

    public void Destroy()
    {
        for (int i = 0; i < _commandAllocators.Length; ++i)
        {
            _commandAllocators[i].Dispose();
        }

        _commandList.Dispose();
    }

    public ID3D12CommandList* End()
    {
        //for (auto & surface : presentSurfaces)
        //{
        //    TextureBarrier(surface->backbufferTextures[surface->backBufferIndex], TextureLayout::Present);
        //}
        CommitBarriers();

        if (_hasLabel)
        {
            PopDebugGroup();
        }

        ThrowIfFailed(_commandList.Get()->Close());

        return (ID3D12CommandList*)_commandList.Get();
    }

    public void Begin(uint frameIndex, Utf8ReadOnlyString label = default)
    {
        base.Reset(frameIndex);
        _currentPipelineLayout = default;
        _bindGroupsDirty = false;
        _numBoundBindGroups = 0;
        Array.Clear(_boundBindGroups, 0, _boundBindGroups.Length);

        // Start the command list in a default state:
        ThrowIfFailed(_commandAllocators[frameIndex].Get()->Reset());
        ThrowIfFailed(_commandList.Get()->Reset(_commandAllocators[frameIndex].Get(), null));

        if (!label.IsEmpty)
        {
            _hasLabel = true;
            PushDebugGroup(label);
        }

        if (_queue.QueueType != CommandQueueType.Copy)
        {
            ID3D12DescriptorHeap** descriptorHeaps = stackalloc ID3D12DescriptorHeap*[2]
            {
                _queue.D3DDevice.ShaderResourceViewHeap.ShaderVisibleHeap,
                _queue.D3DDevice.SamplerHeap.ShaderVisibleHeap,
            };
            _commandList.Get()->SetDescriptorHeaps(2, descriptorHeaps);
        }

        if (_queue.QueueType == CommandQueueType.Graphics)
        {

            RECT* scissorRects = stackalloc RECT[D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX + 1];
            for (int i = 0; i < D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX; ++i)
            {
                scissorRects[i].bottom = D3D12_VIEWPORT_BOUNDS_MAX;
                scissorRects[i].left = D3D12_VIEWPORT_BOUNDS_MIN;
                scissorRects[i].right = D3D12_VIEWPORT_BOUNDS_MAX;
                scissorRects[i].top = D3D12_VIEWPORT_BOUNDS_MIN;
            }
            _commandList.Get()->RSSetScissorRects(D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX, scissorRects);
        }
    }

    public void EndEncoding()
    {
        _encoderActive = false;
    }

    public void TextureBarrier(D3D12Texture resource, TextureLayout newLayout, uint subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool commit = false)
    {
        uint index = (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) ? 0 : subresource;
        TextureLayout currentLayout = resource.GetTextureLayout(index);
        if (currentLayout == newLayout)
            return;

        if (_queue.D3DDevice.DxAdapter.Features.EnhancedBarriersSupported)
        {
        }

        D3D12_RESOURCE_STATES oldStateLegacy = ConvertTextureLayoutLegacy(currentLayout);
        D3D12_RESOURCE_STATES newStateLegacy = ConvertTextureLayoutLegacy(newLayout);

        if (_queue.QueueType == CommandQueueType.Compute)
        {
            Guard.IsTrue((oldStateLegacy & s_ValidComputeResourceStates) == oldStateLegacy);
            Guard.IsTrue((newStateLegacy & s_ValidComputeResourceStates) == newStateLegacy);
        }
        else if (_queue.QueueType == CommandQueueType.Copy)
        {
            Guard.IsTrue((oldStateLegacy & s_ValidCopyResourceStates) == oldStateLegacy);
            Guard.IsTrue((newStateLegacy & s_ValidCopyResourceStates) == newStateLegacy);
        }

        if (oldStateLegacy != newStateLegacy)
        {
            Guard.IsTrue(_numBarriersToFlush < MaxBarriers, "Exceeded arbitrary limit on buffered barriers");

            ref D3D12_RESOURCE_BARRIER barrierDesc = ref _resourceBarriers[_numBarriersToFlush++];
            barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrierDesc.Transition.pResource = resource.Handle;
            barrierDesc.Transition.Subresource = subresource; // D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrierDesc.Transition.StateBefore = oldStateLegacy;
            barrierDesc.Transition.StateAfter = newStateLegacy;

            // Check to see if we already started the transition
            //if (NewState == Resource.m_TransitioningState)
            //{
            //    BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            //    Resource.m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
            //}
            //else
            {
                barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            }

            resource.SetTextureLayout(index, newLayout);
        }
        else if (newStateLegacy == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            InsertUAVBarrier(resource, commit);
        }

        if (commit || _numBarriersToFlush == MaxBarriers)
        {
            CommitBarriers();
        }
    }

    public void InsertUAVBarrier(ID3D12GpuResource resource, bool commit = false)
    {
        Guard.IsTrue(_numBarriersToFlush < _resourceBarriers.Length, "Exceeded arbitrary limit on buffered barriers");
        ref D3D12_RESOURCE_BARRIER barrierDesc = ref _resourceBarriers[_numBarriersToFlush++];

        barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrierDesc.UAV.pResource = resource.Handle;

        if (commit)
            CommitBarriers();
    }

    public void CommitBarriers()
    {
        if (_numBarriersToFlush > 0)
        {
            fixed (D3D12_RESOURCE_BARRIER* pBarriers = _resourceBarriers)
            {
                _commandList.Get()->ResourceBarrier(_numBarriersToFlush, pBarriers);
            }

            _numBarriersToFlush = 0;
        }
    }

    public override void PushDebugGroup(Utf8ReadOnlyString groupLabel)
    {
        // TODO: Use Pix3 (WinPixEventRuntime)

        int bufferSize = PixHelpers.CalculateNoArgsEventSize(groupLabel);
        byte* buffer = stackalloc byte[bufferSize];
        PixHelpers.FormatNoArgsEventToBuffer(buffer, PixHelpers.PixEventType.PIXEvent_BeginEvent_NoArgs, 0, groupLabel);
        _commandList.Get()->BeginEvent(PixHelpers.WinPIXEventPIX3BlobVersion, buffer, (uint)bufferSize);
    }

    public override void PopDebugGroup()
    {
        _commandList.Get()->EndEvent();
    }

    public override void InsertDebugMarker(Utf8ReadOnlyString debugLabel)
    {
        int bufferSize = PixHelpers.CalculateNoArgsEventSize(debugLabel);
        byte* buffer = stackalloc byte[bufferSize];
        PixHelpers.FormatNoArgsEventToBuffer(buffer, PixHelpers.PixEventType.PIXEvent_SetMarker_NoArgs, 0, debugLabel);
        _commandList.Get()->SetMarker(PixHelpers.WinPIXEventPIX3BlobVersion, buffer, (uint)bufferSize);
    }
    public void SetPipelineLayout(D3D12PipelineLayout newPipelineLayout)
    {
        if (_currentPipelineLayout == newPipelineLayout)
            return;

        _currentPipelineLayout = newPipelineLayout;
        //_currentPipelineLayout.AddRef();
    }

    public void SetBindGroup(int groupIndex, BindGroup bindGroup)
    {
        if (_boundBindGroups[groupIndex] != bindGroup)
        {
            _bindGroupsDirty = true;
            _boundBindGroups[groupIndex] = (D3D12BindGroup)bindGroup;
            _numBoundBindGroups = Math.Max(groupIndex + 1, _numBoundBindGroups);
        }
    }

    protected override ComputePassEncoder BeginComputePassCore(in ComputePassDescriptor descriptor)
    {
        _computePassEncoder.Begin(in descriptor);
        return _computePassEncoder;
    }

    protected override RenderPassEncoder BeginRenderPassCore(in RenderPassDescriptor descriptor)
    {
        _renderPassEncoder.Begin(in descriptor);
        return _renderPassEncoder;
    }

    public void FlushBindGroups(bool graphics)
    {
        Debug.Assert(_currentPipelineLayout != null);

        if (!_bindGroupsDirty)
            return;

        for (int groupIndex = 0; groupIndex < _currentPipelineLayout.BindGroupLayoutCount; groupIndex++)
        {
            Debug.Assert(_boundBindGroups[groupIndex] != null);

            D3D12BindGroup bindGroup = _boundBindGroups[groupIndex];

            if (_currentPipelineLayout.DescriptorTableValidCbvUavSrv(groupIndex))
            {
                uint rootParameterIndex = _currentPipelineLayout.GetCbvUavSrvRootParameterIndex(groupIndex);
                D3D12_GPU_DESCRIPTOR_HANDLE gpuHadle = _queue.D3DDevice.ShaderResourceViewHeap.GetGpuHandle(bindGroup.DescriptorTableCbvUavSrv);

                if (graphics)
                {
                    _commandList.Get()->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
                else
                {
                    _commandList.Get()->SetComputeRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
            }

            if (_currentPipelineLayout.DescriptorTableValidSamplers(groupIndex))
            {
                uint rootParameterIndex = _currentPipelineLayout.GetSamplerRootParameterIndex(groupIndex);
                D3D12_GPU_DESCRIPTOR_HANDLE gpuHadle = _queue.D3DDevice.SamplerHeap.GetGpuHandle(bindGroup.DescriptorTableSamplers);

                if (graphics)
                {
                    _commandList.Get()->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
                else
                {
                    _commandList.Get()->SetComputeRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
            }
        }

        _bindGroupsDirty = false;
    }

    public override void Present(SwapChain swapChain)
    {
        var d3dSwapChain = (D3D12SwapChain)swapChain;
        _queue.QueuePresent(d3dSwapChain);
    }


}
