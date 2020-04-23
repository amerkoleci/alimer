// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Collections.Immutable;
using Vortice.DXGI;
using static Vortice.DXGI.DXGI;

namespace Alimer.Graphics.Direct3D11
{
    public sealed class D3D11GraphicsProvider : GraphicsProvider
    {
        public static IDXGIFactory2 _dxgiFactory;
        private ImmutableArray<GraphicsAdapter> _adapters;

        public D3D11GraphicsProvider()
        {
            // Just try to enable debug layer.
            var debugDXGI = false;
            if (EnableValidation
                && DXGIGetDebugInterface1<IDXGIInfoQueue>(out var dxgiInfoQueue).Success)
            {
                debugDXGI = true;

                CreateDXGIFactory2(true, out _dxgiFactory).CheckError();

                dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Error, true);
                dxgiInfoQueue.SetBreakOnSeverity(All, InfoQueueMessageSeverity.Corruption, true);

                dxgiInfoQueue.AddStorageFilterEntries(Dxgi, new InfoQueueFilter
                {
                    DenyList = new InfoQueueFilterDescription
                    {
                        Ids = new[]
                        {
                            80 // IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides
                        }
                    }
                });
            }

            if (!debugDXGI)
            {
                CreateDXGIFactory1(out _dxgiFactory).CheckError();
            }

            var graphicsAdapters = ImmutableArray.CreateBuilder<GraphicsAdapter>();
            foreach (var dxgiAdapter in _dxgiFactory.Adapters1)
            {
                graphicsAdapters.Add(new D3D11GraphicsAdapter(this, dxgiAdapter));
            }

            _adapters = graphicsAdapters.ToImmutable();
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="D3D11GraphicsProvider" /> class.
        /// </summary>
        ~D3D11GraphicsProvider()
        {
            Dispose(disposing: false);
        }

        public override ImmutableArray<GraphicsAdapter> GraphicsAdapters => _adapters;

        /// <inheritdoc />
        protected override void Dispose(bool disposing) => _dxgiFactory?.Dispose();
    }
}
