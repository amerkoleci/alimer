// Copyright © Tanner Gooding and Contributors. Licensed under the MIT License (MIT). See License.md in the repository root for more information.

// Ported from d3dcommon.h in microsoft/DirectX-Headers tag v1.611.2
// Original source is Copyright © Microsoft. Licensed under the MIT license

namespace TerraFX.Interop.DirectX;

internal enum D3D_TESSELLATOR_PARTITIONING
{
    D3D_TESSELLATOR_PARTITIONING_UNDEFINED = 0,

    D3D_TESSELLATOR_PARTITIONING_INTEGER = 1,

    D3D_TESSELLATOR_PARTITIONING_POW2 = 2,

    D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD = 3,

    D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN = 4,
}
