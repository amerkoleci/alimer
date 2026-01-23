// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using CommunityToolkit.Diagnostics;
using TerraFX.Interop.DirectX;
using TerraFX.Interop.Windows;
using static Alimer.Graphics.Constants;
using static Alimer.Graphics.D3D12.D3D12Utils;
using static TerraFX.Interop.DirectX.D3D12;
using static TerraFX.Interop.DirectX.D3D12_COMMAND_LIST_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_BARRIER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BARRIER_FLAGS;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_BARRIER_TYPE;
using static TerraFX.Interop.DirectX.D3D12_RESOURCE_STATES;
using static TerraFX.Interop.DirectX.D3D12_TEXTURE_BARRIER_FLAGS;
using static TerraFX.Interop.Windows.Windows;
using Alimer.Utilities;
using System.Runtime.CompilerServices;

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
    private readonly ComPtr<ID3D12GraphicsCommandList6> _commandList6;
    private readonly ComPtr<ID3D12GraphicsCommandList7> _commandList7;

    private readonly D3D12RenderPassEncoder _renderPassEncoder;
    private readonly D3D12ComputePassEncoder _computePassEncoder;

    private uint _numBarriersToFlush;
    private uint _textureBarriersCount;
    private readonly D3D12_RESOURCE_BARRIER[] _resourceBarriers = new D3D12_RESOURCE_BARRIER[MaxBarriers];
    //private readonly D3D12_GLOBAL_BARRIER[] _globalBarriers = new D3D12_GLOBAL_BARRIER[MaxBarriers];
    private readonly D3D12_TEXTURE_BARRIER[] _textureBarriers = new D3D12_TEXTURE_BARRIER[MaxBarriers];
    //private readonly D3D12_BUFFER_BARRIER[] _bufferBarriers = new D3D12_BUFFER_BARRIER[MaxBarriers];

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
            __uuidof<ID3D12GraphicsCommandList6>(), (void**)_commandList6.GetAddressOf()
        ));
        _commandList6.CopyTo(_commandList7.GetAddressOf());

        _renderPassEncoder = new D3D12RenderPassEncoder(this);
        _computePassEncoder = new D3D12ComputePassEncoder(this);
    }

    /// <inheritdoc />
    public override GraphicsDevice Device => _queue.Device;

    public D3D12GraphicsDevice D3DDevice => _queue.D3DDevice;
    public ID3D12GraphicsCommandList6* CommandList => _commandList6;

    public void Destroy()
    {
        for (int i = 0; i < _commandAllocators.Length; ++i)
        {
            _commandAllocators[i].Dispose();
        }

        _commandList7.Dispose();
        _commandList6.Dispose();
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

        ThrowIfFailed(_commandList6.Get()->Close());

        return (ID3D12CommandList*)_commandList6.Get();
    }

    public void Begin(uint frameIndex, Utf8ReadOnlyString label = default)
    {
        base.Reset(frameIndex);
        _currentPipelineLayout = default;
        _bindGroupsDirty = false;
        _numBoundBindGroups = 0;
        _numBarriersToFlush = 0;
        _textureBarriersCount = 0;
        Array.Clear(_textureBarriers, 0, _textureBarriers.Length);
        Array.Clear(_boundBindGroups, 0, _boundBindGroups.Length);

        // Start the command list in a default state:
        ThrowIfFailed(_commandAllocators[frameIndex].Get()->Reset());
        ThrowIfFailed(_commandList6.Get()->Reset(_commandAllocators[frameIndex].Get(), null));

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
            _commandList6.Get()->SetDescriptorHeaps(2, descriptorHeaps);
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
            _commandList6.Get()->RSSetScissorRects(D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX, scissorRects);
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

        if (_queue.D3DDevice.EnhancedBarriersSupported)
        {
            D3D12TextureLayoutMapping mappingBefore = ConvertTextureLayout(currentLayout);
            D3D12TextureLayoutMapping mappingAfter = ConvertTextureLayout(newLayout);

            Guard.IsTrue(_textureBarriersCount < MaxBarriers, "Exceeded arbitrary limit on texture barriers");

            ref D3D12_TEXTURE_BARRIER barrier = ref _textureBarriers[_textureBarriersCount++];

            barrier.SyncBefore = mappingBefore.Sync;
            barrier.SyncAfter = mappingAfter.Sync;
            barrier.AccessBefore = mappingBefore.Access;
            barrier.AccessAfter = mappingAfter.Access;
            barrier.LayoutBefore = mappingBefore.Layout;
            barrier.LayoutAfter = mappingAfter.Layout;
            barrier.pResource = resource.Handle;
            if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                barrier.Subresources.IndexOrFirstMipLevel = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Subresources.NumMipLevels = 0;
                barrier.Subresources.FirstArraySlice = 0;
                barrier.Subresources.NumArraySlices = 0;
                barrier.Subresources.FirstPlane = 0;
                barrier.Subresources.NumPlanes = 0;
            }
            else
            {
                // TODO:
            }
            barrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE; // TODO Handle discard when we have transient resources
            Debug.Assert(barrier.LayoutBefore != barrier.LayoutAfter);

            commit = _textureBarriersCount == MaxBarriers;
            resource.SetTextureLayout(index, newLayout);
        }
        else
        {
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
        //if (!globalBarriers.empty() || !textureBarriers.empty() || !bufferBarriers.empty())
        if (_textureBarriersCount > 0)
        {
            uint numBarrierGroups = 0;
            D3D12_BARRIER_GROUP* barrierGroups = stackalloc D3D12_BARRIER_GROUP[3];

            //if (!globalBarriers.empty())
            //{
            //    barrierGroups[numBarrierGroups].Type = D3D12_BARRIER_TYPE_GLOBAL;
            //    barrierGroups[numBarrierGroups].NumBarriers = (UINT32)globalBarriers.size();
            //    barrierGroups[numBarrierGroups].pGlobalBarriers = globalBarriers.data();
            //    numBarrierGroups++;
            //}

            if (_textureBarriersCount > 0)
            {
                barrierGroups[numBarrierGroups].Type = D3D12_BARRIER_TYPE_TEXTURE;
                barrierGroups[numBarrierGroups].NumBarriers = _textureBarriersCount;
                barrierGroups[numBarrierGroups].pTextureBarriers = (D3D12_TEXTURE_BARRIER*)Unsafe.AsPointer(ref UnsafeUtilities.GetReferenceUnsafe(_textureBarriers));
                numBarrierGroups++;
            }

            //if (!bufferBarriers.empty())
            //{
            //    barrierGroups[numBarrierGroups].Type = D3D12_BARRIER_TYPE_BUFFER;
            //    barrierGroups[numBarrierGroups].NumBarriers = (UINT32)bufferBarriers.size();
            //    barrierGroups[numBarrierGroups].pBufferBarriers = bufferBarriers.data();
            //    numBarrierGroups++;
            //}

            _commandList7.Get()->Barrier(numBarrierGroups, barrierGroups);

            //globalBarriers.clear();
            _textureBarriersCount = 0;
            //bufferBarriers.clear();
        }
        else if (_numBarriersToFlush > 0)
        {
            fixed (D3D12_RESOURCE_BARRIER* pBarriers = _resourceBarriers)
            {
                _commandList6.Get()->ResourceBarrier(_numBarriersToFlush, pBarriers);
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
        _commandList6.Get()->BeginEvent(PixHelpers.WinPIXEventPIX3BlobVersion, buffer, (uint)bufferSize);
    }

    public override void PopDebugGroup()
    {
        _commandList6.Get()->EndEvent();
    }

    public override void InsertDebugMarker(Utf8ReadOnlyString debugLabel)
    {
        int bufferSize = PixHelpers.CalculateNoArgsEventSize(debugLabel);
        byte* buffer = stackalloc byte[bufferSize];
        PixHelpers.FormatNoArgsEventToBuffer(buffer, PixHelpers.PixEventType.PIXEvent_SetMarker_NoArgs, 0, debugLabel);
        _commandList6.Get()->SetMarker(PixHelpers.WinPIXEventPIX3BlobVersion, buffer, (uint)bufferSize);
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
                    _commandList6.Get()->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
                else
                {
                    _commandList6.Get()->SetComputeRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
            }

            if (_currentPipelineLayout.DescriptorTableValidSamplers(groupIndex))
            {
                uint rootParameterIndex = _currentPipelineLayout.GetSamplerRootParameterIndex(groupIndex);
                D3D12_GPU_DESCRIPTOR_HANDLE gpuHadle = _queue.D3DDevice.SamplerHeap.GetGpuHandle(bindGroup.DescriptorTableSamplers);

                if (graphics)
                {
                    _commandList6.Get()->SetGraphicsRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
                else
                {
                    _commandList6.Get()->SetComputeRootDescriptorTable(rootParameterIndex, gpuHadle);
                }
            }
        }

        _bindGroupsDirty = false;
    }

    public override void Present(SwapChain swapChain)
    {
        D3D12SwapChain backendSwapChain = (D3D12SwapChain)swapChain;
        //TextureBarrier(backendSwapChain.CurrentTexture, TextureLayout.Present, 0, 1, 0, 1);
        TextureBarrier(backendSwapChain.CurrentTexture, TextureLayout.Present);
        _queue.QueuePresent(backendSwapChain);
    }
}
