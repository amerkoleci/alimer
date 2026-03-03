// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Numerics;

public static class Extensions
{
    extension(Vector3)
    {
        /// <summary>
        /// (-1,0,0) vector.
        /// </summary>
        public static Vector3 Left => new(-1.0f, 0.0f, 0.0f);

        /// <summary>
        /// (1,0,0) vector.
        /// </summary>
        public static Vector3 Right => new(1.0f, 0.0f, 0.0f);
        /// <summary>
        /// (0,1,0) vector.
        /// </summary>
        public static Vector3 Up => new(0.0f, 1.0f, 0.0f);
        /// <summary>
        /// (0,-1,0) vector.
        /// </summary>
        public static Vector3 Down => new(0.0f, -1.0f, 0.0f);
        /// <summary>
        /// (0,0,-1) vector designating forward in the default (right-handed coordinate) system.
        /// </summary>
        public static Vector3 Forward => new(0.0f, 0.0f, -1.0f);
        /// <summary>
        /// (0,0,1) vector designating backward in the default (right-handed coordinate) system.
        /// </summary>
        public static Vector3 Backward => new(0.0f, 0.0f, 1.0f);
    }
}
