// Copyright (c) Amer Koleci and contributors.
// Distributed under the MIT license. See the LICENSE file in the project root for more information.

using System.Runtime.InteropServices;

namespace Vortice.Graphics
{
    public enum LoadOp
    {
        Clear = 0,
        Load = 1,
    }

    public struct Color4
    {
        public float R;
        public float G;
        public float B;
        public float A;

        public Color4(float r, float g, float b, float a = 1.0f)
        {
            R = r;
            G = g;
            B = b;
            A = a;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RenderPassColorAttachment
    {
        public GPUTexture texture;
        public uint mipLevel;
        public uint slice;
        public LoadOp loadOp;
        public Color4 clearColor;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RenderPassDepthStencilAttachment
    {
        public GPUTexture texture;
        public LoadOp depthLoadOp;
        public float clearDepth;
        public LoadOp stencilLoadOp;
        public uint clearStencil;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct RenderPassDescription
    {
        public RenderPassColorAttachment colorAttachments0;
        public RenderPassColorAttachment colorAttachments1;
        public RenderPassColorAttachment colorAttachments2;
        public RenderPassColorAttachment colorAttachments3;
        public RenderPassColorAttachment colorAttachments4;
        public RenderPassColorAttachment colorAttachments5;
        public RenderPassColorAttachment colorAttachments6;
        public RenderPassColorAttachment colorAttachments7;
        public RenderPassDepthStencilAttachment depthStencilAttachment;
    }
}
