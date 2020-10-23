// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Alimer.Graphics
{
    [StructLayout(LayoutKind.Sequential)]
    public readonly struct GPUDeviceFeatures
    {
        public readonly Bool32 IndependentBlend;
        public readonly Bool32 ComputeShader;
        public readonly Bool32 TessellationShader;
        public readonly Bool32 MultiViewport;
        public readonly Bool32 IndexUInt32;
        public readonly Bool32 MultiDrawIndirect;
        public readonly Bool32 FillModeNonSolid;
        public readonly Bool32 SamplerAnisotropy;
        public readonly Bool32 TextureCompressionETC2;
        public readonly Bool32 TextureCompressionASTC_LDR;
        public readonly Bool32 TextureCompressionBC;
        public readonly Bool32 TextureCubeArray;
        public readonly Bool32 Raytracing;
    }
}
