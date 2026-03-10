// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Diagnostics;
using Alimer.Assets.Graphics;
using Alimer.Graphics;
using Alimer.Serialization;

namespace Alimer.Assets.Writers;

internal class ShaderWriter : AssetTypeWriter<ShaderAsset>
{
    public override string FileExtension => "ashd";
    public override string MagicNumber => "ASHD";

    public override void Write(ref WriteByteStream writer, ShaderAsset asset)
    {
    }
}
