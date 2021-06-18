// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Vortice.Graphics
{
    public struct GraphicsDeviceFeatures
    {
        public bool IndependentBlend;
        public bool ComputeShader;
        public bool TessellationShader;
        public bool MultiViewport;
        public bool IndexUInt32;
        public bool MultiDrawIndirect;
        public bool FillModeNonSolid;
        public bool SamplerAnisotropy;
        public bool TextureCompressionETC2;
        public bool TextureCompressionASTC_LDR;
        public bool TextureCompressionBC;
        public bool TextureCubeArray;
        public bool Raytracing;
    }
}
