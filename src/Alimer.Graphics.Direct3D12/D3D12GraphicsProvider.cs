// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;
using static Vortice.Direct3D12.D3D12;
using Vortice.Direct3D12.Debug;
using System.Collections.Immutable;

namespace Alimer.Graphics.Direct3D12
{
    public sealed class D3D12GraphicsProvider : GraphicsProvider
    {
        public static IDXGIFactory4 _dxgiFactory;
        private ImmutableArray<GraphicsAdapter> _adapters;

        public D3D12GraphicsProvider()
        {
            var validation = false;
            if (EnableValidation
                && D3D12GetDebugInterface<ID3D12Debug>(out var debugController).Success)
            {
                // Enable the D3D12 debug layer.
                debugController.EnableDebugLayer();

                var debugController1 = debugController.QueryInterfaceOrNull<ID3D12Debug1>();
                if (debugController1 != null)
                {
                    if (EnableGPUBasedValidation)
                    {
                        debugController1.SetEnableGPUBasedValidation(true);
                        debugController1.SetEnableSynchronizedCommandQueueValidation(true);
                    }
                    else
                    {
                        debugController1.SetEnableGPUBasedValidation(false);
                    }
                    debugController1.Dispose();
                }

                debugController.Dispose();

                if (DXGIGetDebugInterface1<IDXGIInfoQueue>(out var dxgiInfoQueue).Success)
                {
                    validation = true;
                    dxgiInfoQueue.Dispose();
                }
            }

            if (CreateDXGIFactory2(validation, out _dxgiFactory).Failure)
            {
            }

            var graphicsAdapters = ImmutableArray.CreateBuilder<GraphicsAdapter>();
            foreach (var dxgiAdapter in _dxgiFactory.Adapters1)
            {
                graphicsAdapters.Add(new D3D12GraphicsAdapter(this, dxgiAdapter));
            }

            _adapters = graphicsAdapters.ToImmutable();
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D12GraphicsProvider" /> class.
        /// </summary>
        ~D3D12GraphicsProvider()
        {
            Dispose(disposing: false);
        }

        public override ImmutableArray<GraphicsAdapter> GraphicsAdapters => _adapters;

        /// <inheritdoc />
        protected override void Dispose(bool disposing)
        {
            if (_adapters.IsDefault)
            {
                foreach (GraphicsAdapter adapter in _adapters)
                {
                    adapter.Dispose();
                }
            }

            _dxgiFactory?.Dispose();
        }
    }
}
