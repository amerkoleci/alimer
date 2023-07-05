// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Drawing;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Numerics;

namespace Alimer.Graphics;

/// <summary>
/// Represents a floating-point viewport struct.
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 4)]
public struct Viewport : IEquatable<Viewport>
{
    /// <summary>
    /// Position of the pixel coordinate of the upper-left corner of the viewport.
    /// </summary>
    public float X;

    /// <summary>
    /// Position of the pixel coordinate of the upper-left corner of the viewport.
    /// </summary>
    public float Y;

    /// <summary>
    /// Width dimension of the viewport.
    /// </summary>
    public float Width;

    /// <summary>
    /// Height dimension of the viewport.
    /// </summary>
    public float Height;

    /// <summary>
    /// Gets or sets the minimum depth of the clip volume.
    /// </summary>
    public float MinDepth;

    /// <summary>
    /// Gets or sets the maximum depth of the clip volume.
    /// </summary>
    public float MaxDepth;

    /// <summary>
    /// Initializes a new instance of the <see cref="Viewport"/> struct.
    /// </summary>
    /// <param name="width">The width of the viewport in pixels.</param>
    /// <param name="height">The height of the viewport in pixels.</param>
    public Viewport(float width, float height)
    {
        X = 0.0f;
        Y = 0.0f;
        Width = width;
        Height = height;
        MinDepth = 0.0f;
        MaxDepth = 1.0f;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Viewport"/> struct.
    /// </summary>
    /// <param name="x">The x coordinate of the upper-left corner of the viewport in pixels.</param>
    /// <param name="y">The y coordinate of the upper-left corner of the viewport in pixels.</param>
    /// <param name="width">The width of the viewport in pixels.</param>
    /// <param name="height">The height of the viewport in pixels.</param>
    public Viewport(float x, float y, float width, float height)
    {
        X = x;
        Y = y;
        Width = width;
        Height = height;
        MinDepth = 0.0f;
        MaxDepth = 1.0f;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Viewport"/> struct.
    /// </summary>
    /// <param name="x">The x coordinate of the upper-left corner of the viewport in pixels.</param>
    /// <param name="y">The y coordinate of the upper-left corner of the viewport in pixels.</param>
    /// <param name="width">The width of the viewport in pixels.</param>
    /// <param name="height">The height of the viewport in pixels.</param>
    /// <param name="minDepth">The minimum depth of the clip volume.</param>
    /// <param name="maxDepth">The maximum depth of the clip volume.</param>
    public Viewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        X = x;
        Y = y;
        Width = width;
        Height = height;
        MinDepth = minDepth;
        MaxDepth = maxDepth;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Viewport"/> struct.
    /// </summary>
    /// <param name="bounds">A <see cref="RectangleF"/> that defines the location and size of the viewport in a render target.</param>
    public Viewport(RectangleF bounds)
    {
        X = bounds.X;
        Y = bounds.Y;
        Width = bounds.Width;
        Height = bounds.Height;
        MinDepth = 0.0f;
        MaxDepth = 1.0f;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Viewport"/> struct.
    /// </summary>
    /// <param name="bounds">A <see cref="Rectangle"/> that defines the location and size of the viewport in a render target.</param>
    public Viewport(Rectangle bounds)
    {
        X = bounds.X;
        Y = bounds.Y;
        Width = bounds.Width;
        Height = bounds.Height;
        MinDepth = 0.0f;
        MaxDepth = 1.0f;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="Viewport"/> struct.
    /// </summary>
    /// <param name="bounds">A <see cref="Vector4"/> that defines the location and size of the viewport in a render target.</param>
    public Viewport(in Vector4 bounds)
    {
        X = bounds.X;
        Y = bounds.Y;
        Width = bounds.Z;
        Height = bounds.W;
        MinDepth = 0.0f;
        MaxDepth = 1.0f;
    }

    /// <summary>
    /// Gets or sets the bounds of the viewport.
    /// </summary>
    /// <value>The bounds.</value>
    public readonly RectangleF Bounds => new(X, Y, Width, Height);

    /// <summary>
    /// Gets the aspect ratio used by the viewport.
    /// </summary>
    /// <value>The aspect ratio.</value>
    public readonly float AspectRatio
    {
        get
        {
            if (Width != 0 && Height != 0)
            {
                return Width / Height;
            }

            return 0.0f;
        }
    }

    /// <summary>
    /// Projects a 3D vector from object space into screen space.
    /// </summary>
    /// <param name="source">The vector to project.</param>
    /// <param name="projection">The projection matrix.</param>
    /// <param name="view">The view matrix.</param>
    /// <param name="world">The world matrix.</param>
    public Vector3 Project(in Vector3 source, in Matrix4x4 projection, in Matrix4x4 view, in Matrix4x4 world)
    {
        var worldViewProjection = Matrix4x4.Multiply(Matrix4x4.Multiply(world, view), projection);
        return Project(source, worldViewProjection);
    }

    /// <summary>
    /// Projects a 3D vector from object space into screen space.
    /// </summary>
    /// <param name="source">The vector to project.</param>
    /// <param name="worldViewProjection">The World-View-Projection matrix.</param>
    /// <returns>The unprojected vector. </returns>
    public Vector3 Project(in Vector3 source, in Matrix4x4 worldViewProjection)
    {
        var vector = Vector3.Transform(source, worldViewProjection);
        var a = (source.X * worldViewProjection.M14) + (source.Y * worldViewProjection.M24) + (source.Z * worldViewProjection.M34) + worldViewProjection.M44;

        if (!MathHelper.IsOne(a))
        {
            vector.X /= a;
            vector.Y /= a;
            vector.Z /= a;
        }

        vector.X = ((vector.X + 1.0f) * 0.5f * Width) + X;
        vector.Y = ((-vector.Y + 1.0f) * 0.5f * Height) + Y;
        vector.Z = (vector.Z * (MaxDepth - MinDepth)) + MinDepth;
        return vector;
    }

    /// <summary>
    /// Converts a screen space point into a corresponding point in world space.
    /// </summary>
    /// <param name="source">The vector to project.</param>
    /// <param name="projection">The projection matrix.</param>
    /// <param name="view">The view matrix.</param>
    /// <param name="world">The world matrix.</param>
    /// <returns>The unprojected vector. </returns>
    public Vector3 Unproject(in Vector3 source, in Matrix4x4 projection, in Matrix4x4 view, in Matrix4x4 world)
    {
        var worldViewProjection = Matrix4x4.Multiply(Matrix4x4.Multiply(world, view), projection);
        return Unproject(source, worldViewProjection);
    }

    /// <summary>
    /// Converts a screen space point into a corresponding point in world space.
    /// </summary>
    /// <param name="source">The vector to project.</param>
    /// <param name="worldViewProjection">The World-View-Projection matrix.</param>
    /// <returns>The unprojected vector. </returns>
    public Vector3 Unproject(Vector3 source, in Matrix4x4 worldViewProjection)
    {
        Matrix4x4.Invert(worldViewProjection, out var matrix);

        source.X = (((source.X - X) / Width) * 2.0f) - 1.0f;
        source.Y = -(((source.Y - Y) / Height * 2.0f) - 1.0f);
        source.Z = (source.Z - MinDepth) / (MaxDepth - MinDepth);

        float a = (source.X * matrix.M14) + (source.Y * matrix.M24) + (source.Z * matrix.M34) + matrix.M44;
        source = Vector3.Transform(source, matrix);

        if (!MathHelper.IsOne(a))
        {
            source /= a;
        }

        return source;
    }

    /// <inheritdoc/>
    public override bool Equals(object? obj) => obj is Viewport value && Equals(value);

    /// <summary>
    /// Determines whether the specified <see cref="Viewport"/> is equal to this instance.
    /// </summary>
    /// <param name="other">The <see cref="Viewport"/> to compare with this instance.</param>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public bool Equals(Viewport other)
    {
        return X == other.X
            && Y == other.Y
            && Width == other.Width
            && Height == other.Height
            && MinDepth == other.MinDepth
            && MaxDepth == other.MaxDepth;
    }

    /// <summary>
    /// Compares two <see cref="Viewport"/> objects for equality.
    /// </summary>
    /// <param name="left">The <see cref="Viewport"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Viewport"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is equal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator ==(Viewport left, Viewport right) => left.Equals(right);

    /// <summary>
    /// Compares two <see cref="Viewport"/> objects for inequality.
    /// </summary>
    /// <param name="left">The <see cref="Viewport"/> on the left hand of the operand.</param>
    /// <param name="right">The <see cref="Viewport"/> on the right hand of the operand.</param>
    /// <returns>
    /// True if the current left is unequal to the <paramref name="right"/> parameter; otherwise, false.
    /// </returns>
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static bool operator !=(Viewport left, Viewport right) => !left.Equals(right);

    /// <inheritdoc/>
    public override int GetHashCode()
    {
        var hashCode = new HashCode();
        {
            hashCode.Add(X);
            hashCode.Add(Y);
            hashCode.Add(Width);
            hashCode.Add(Height);
            hashCode.Add(MinDepth);
            hashCode.Add(MaxDepth);
        }
        return hashCode.ToHashCode();
    }

    /// <inheritdoc/>
    public override string ToString()
    {
        return $"{nameof(X)}: {X}, {nameof(Y)}: {Y}, {nameof(Width)}: {Width}, {nameof(Height)}: {Height}, {nameof(MinDepth)}: {MinDepth}, {nameof(MaxDepth)}: {MaxDepth}";
    }
}
