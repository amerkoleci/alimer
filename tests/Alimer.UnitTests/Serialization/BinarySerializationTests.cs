// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;
using NUnit.Framework;

namespace Alimer.Serialization;

[TestFixture(TestOf = typeof(BinarySerializer))]
public class BinarySerializationTests
{
    [Theory]
    public void TestPrimitives()
    {
        byte testByte = byte.MaxValue;
        sbyte testSByte = sbyte.MinValue;
        bool testBool = true;
        short testShort = short.MinValue;
        ushort testUShort = ushort.MaxValue;
        int testInt = int.MinValue;
        uint testUInt = uint.MaxValue;
        long testInt64 = long.MinValue;
        ulong testUInt64 = ulong.MaxValue;
        float testFloat = float.MinValue;
        double testDouble = double.MaxValue;
        string testString = "HELLO WORLD";

        using MemoryStream stream = new();
        BinarySerializer serializer = new(stream, SerializerMode.Write);
        serializer.Serialize(ref testByte);
        serializer.Serialize(ref testSByte);
        serializer.Serialize(ref testBool);
        serializer.Serialize(ref testShort);
        serializer.Serialize(ref testUShort);
        serializer.Serialize(ref testInt);
        serializer.Serialize(ref testUInt);
        serializer.Serialize(ref testInt64);
        serializer.Serialize(ref testUInt64);
        serializer.Serialize(ref testFloat);
        serializer.Serialize(ref testDouble);
        serializer.Serialize(ref testString!);

        // Reset Stream
        stream.Position = 0;

        // Deserialize
        serializer.Mode = SerializerMode.Read;
        serializer.Serialize(ref testByte);
        serializer.Serialize(ref testSByte);
        serializer.Serialize(ref testBool);
        serializer.Serialize(ref testShort);
        serializer.Serialize(ref testUShort);
        serializer.Serialize(ref testInt);
        serializer.Serialize(ref testUInt);
        serializer.Serialize(ref testInt64);
        serializer.Serialize(ref testUInt64);
        serializer.Serialize(ref testFloat);
        serializer.Serialize(ref testDouble);
        serializer.Serialize(ref testString!);

        // Compare
        Assert.That(() => testByte, Is.EqualTo(byte.MaxValue));
        Assert.That(() => testSByte, Is.EqualTo(sbyte.MinValue));
        Assert.That(() => testBool, Is.True);
        Assert.That(() => testShort, Is.EqualTo(short.MinValue));
        Assert.That(() => testUShort, Is.EqualTo(ushort.MaxValue));
        Assert.That(() => testInt, Is.EqualTo(int.MinValue));
        Assert.That(() => testUInt, Is.EqualTo(uint.MaxValue));
        Assert.That(() => testInt64, Is.EqualTo(long.MinValue));
        Assert.That(() => testUInt64, Is.EqualTo(ulong.MaxValue));
        Assert.That(() => testFloat, Is.EqualTo(float.MinValue));
        Assert.That(() => testDouble, Is.EqualTo(double.MaxValue));
        Assert.That(() => testString, Is.EqualTo("HELLO WORLD"));
    }

    [Theory]
    public void TestMathematics()
    {
        Vector2 vector2TestValue = new(256.34f, -120.0f);
        Vector3 vector3TestValue = new(150.5f, -120.0f, 299);
        Vector4 vector4TestValue = new(256.34f, -120.0f, float.MaxValue, float.MinValue);
        Quaternion quatTestValue = new(256.34f, -120.0f, float.MaxValue, float.MinValue);
        Matrix3x2 matrix3X2TestValue = new(256.34f, -120.0f, float.MaxValue, float.MinValue, 0, 1);
        Matrix4x4 matrix4X4TestValue = new(256.34f, -120.0f, float.MaxValue, float.MinValue,
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 10, 11, 12);

        Vector2 vector2 = vector2TestValue;
        Vector3 vector3 = vector3TestValue;
        Vector4 vector4 = vector4TestValue;
        Quaternion quat = quatTestValue;
        Matrix3x2 matrix3x2 = matrix3X2TestValue;
        Matrix4x4 matrix4x4 = matrix4X4TestValue;

        using MemoryStream stream = new();
        BinarySerializer serializer = new(stream, SerializerMode.Write);
        serializer.Serialize(ref vector2);
        serializer.Serialize(ref vector3);
        serializer.Serialize(ref vector4);
        serializer.Serialize(ref quat);
        serializer.Serialize(ref matrix3x2);
        serializer.Serialize(ref matrix4x4);

        // Reset Stream
        stream.Position = 0;

        // Deserialize
        serializer.Mode = SerializerMode.Read;
        serializer.Serialize(ref vector2);
        serializer.Serialize(ref vector3);
        serializer.Serialize(ref vector4);
        serializer.Serialize(ref quat);
        serializer.Serialize(ref matrix3x2);
        serializer.Serialize(ref matrix4x4);

        // Compare
        Assert.That(() => vector2TestValue, Is.EqualTo(vector2));
        Assert.That(() => vector3TestValue, Is.EqualTo(vector3));
        Assert.That(() => vector4TestValue, Is.EqualTo(vector4));
        Assert.That(() => quatTestValue, Is.EqualTo(quat));
        Assert.That(() => matrix3x2, Is.EqualTo(matrix3X2TestValue));

    }

    [Theory]
    public void TestBinarySerializable()
    {
        TextureDesc writeDesc = new()
        {
            Width = 1024,
            Height = 1024,
            Depth = 1,
            Format = PixelFormat.RGBA8Unorm
        };

        using MemoryStream stream = new();
        BinarySerializer serializer = new(stream, SerializerMode.Write);
        serializer.Serialize(ref writeDesc);

        // Reset Stream
        stream.Position = 0;

        // Deserialize
        TextureDesc readDesc = new();
        serializer.Mode = SerializerMode.Read;
        serializer.Serialize(ref readDesc);

        // Compare
        Assert.That(() => readDesc.Width, Is.EqualTo(writeDesc.Width));
        Assert.That(() => readDesc.Height, Is.EqualTo(writeDesc.Height));
        Assert.That(() => readDesc.Depth, Is.EqualTo(writeDesc.Depth));
        Assert.That(() => readDesc.Format, Is.EqualTo(writeDesc.Format));
    }

    class TextureDesc : IBinarySerializable
    {
        public uint Width { get; set; }
        public uint Height { get; set; }
        public uint Depth { get; set; }
        public PixelFormat Format { get; set; }

        public void Serialize(BinarySerializer serializer)
        {
            // TODO: Enum

            uint width = Width;
            uint height = Height;
            uint depth = Depth;
            int format = (int)Format;

            serializer.Serialize(ref width);
            serializer.Serialize(ref height);
            serializer.Serialize(ref depth);
            serializer.Serialize(ref format);

            if (serializer.IsReading)
            {
                Width = width;
                Height = height;
                Depth = depth;
                Format = (PixelFormat)format;
            }
        }
    }
}
