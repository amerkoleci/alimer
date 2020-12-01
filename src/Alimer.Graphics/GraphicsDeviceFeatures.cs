// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

namespace Alimer.Graphics
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
