// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Engine;
using JoltPhysicsSharp;
using JoltMotionType = JoltPhysicsSharp.MotionType;

namespace Alimer.Physics;

public static class JoltUtils
{

    public static JoltMotionType ToJolt(this MotionType value)
    {
        return value switch
        {
            MotionType.Static => JoltMotionType.Static,
            MotionType.Dynamic => JoltMotionType.Dynamic,
            MotionType.Kinematic => JoltMotionType.Kinematic,
            _ => throw new NotImplementedException(),
        };
    }
}
