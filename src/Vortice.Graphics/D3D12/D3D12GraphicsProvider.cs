// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System;
using Vortice.Direct3D;
using Vortice.Direct3D12;
using Vortice.DXGI;

namespace Vortice.Graphics.D3D12
{
    public sealed unsafe class D3D12GraphicsProvider : GraphicsProvider
    {
        private static readonly Lazy<bool> s_isSupported = new Lazy<bool>(CheckIsSupported, isThreadSafe: true);
        //private readonly IDXGIFactory4 _factory4;

        public static bool IsSupported() => s_isSupported.Value;

        public D3D12GraphicsProvider(bool validation)
        {
        }

        /// <summary>
        /// Finalizes an instance of the <see cref="VulkanGraphicsProvider" /> class.
        /// </summary>
        ~D3D12GraphicsProvider() => Dispose(isDisposing: false);


        /// <inheritdoc />
        protected override void Dispose(bool isDisposing)
        {
        }

        private static bool CheckIsSupported()
        {
            return Vortice.Direct3D12.D3D12.IsSupported(FeatureLevel.Level_11_0);
        }
    }
}
