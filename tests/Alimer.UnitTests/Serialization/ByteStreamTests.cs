// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using Alimer.Graphics;
using NUnit.Framework;

namespace Alimer.Serialization;

[TestFixture]
public class ByteStreamTests
{
    [Theory]
    public void TestPrimitives()
    {
        WriteByteStream writer = new();

        bool testBool = true;
        byte testByte = byte.MaxValue;
        sbyte testSByte = sbyte.MinValue;
        short testShort = short.MinValue;
        ushort testUShort = ushort.MaxValue;
        int testInt = int.MinValue;
        uint testUInt = uint.MaxValue;
        long testInt64 = long.MinValue;
        ulong testUInt64 = ulong.MaxValue;
        float testFloat = float.MinValue;
        double testDouble = double.MaxValue;
        string testString = "HELLO WORLD";
        Guid testGuid = Guid.NewGuid();
        TimeSpan testTimeSpan = TimeSpan.FromSeconds(65);
        DateTime testDateTime = DateTime.Now;

        writer.Write(testBool);
        writer.Write(testByte);
        writer.Write(testSByte);
        writer.Write(testShort);
        writer.Write(testUShort);
        writer.Write(testInt);
        writer.Write(testUInt);
        writer.Write(testInt64);
        writer.Write(testUInt64);
        writer.Write(testFloat);
        writer.Write(testDouble);
        writer.Write(testString);
        writer.Write(testGuid);
        writer.Write(testTimeSpan);
        writer.Write(testDateTime);

        // Get written span
        ReadOnlySpan<byte> span = writer.WrittenSpan;

        // Read now
        ReadByteStream readStream = new(span);
        testBool = readStream.Read<bool>();
        testByte = readStream.Read<byte>();
        testSByte = readStream.Read<sbyte>();
        testShort = readStream.Read<short>();
        testUShort = readStream.Read<ushort>();
        testInt = readStream.Read<int>();
        testUInt = readStream.Read<uint>();
        testInt64 = readStream.Read<long>();
        testUInt64 = readStream.Read<ulong>();
        testFloat = readStream.Read<float>();
        testDouble = readStream.Read<double>();
        testString = readStream.ReadString()!;
        Guid readGuid = readStream.ReadGuid();
        TimeSpan readTimeSpan = readStream.ReadTimeSpan();
        DateTime readDateTime = readStream.ReadDateTime();

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
        Assert.That(() => readGuid, Is.EqualTo(testGuid));
        Assert.That(() => readTimeSpan, Is.EqualTo(testTimeSpan));
        Assert.That(() => readDateTime, Is.EqualTo(testDateTime));
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

        WriteByteStream writer = new();
        writer.Write(vector2);
        writer.Write(vector3);
        writer.Write(vector4);
        writer.Write(quat);
        writer.Write(matrix3x2);
        writer.Write(matrix4x4);

        // Get written span
        ReadOnlySpan<byte> span = writer.WrittenSpan;

        // Read now
        ReadByteStream readStream = new(span);
        vector2 = readStream.Read<Vector2>();
        vector3 = readStream.Read<Vector3>();
        vector4 = readStream.Read<Vector4>();
        quat = readStream.Read<Quaternion>();
        matrix3x2 = readStream.Read<Matrix3x2>();
        matrix4x4 = readStream.Read<Matrix4x4>();

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

        WriteByteStream writer = new();
        writer.Write(writeDesc);

        // Get written span
        ReadOnlySpan<byte> span = writer.WrittenSpan;

        // Read now
        ReadByteStream readStream = new(span);

        // Deserialize
        TextureDesc readDesc = readStream.ReadSerializable<TextureDesc>();

        // Compare
        Assert.That(() => readDesc.Width, Is.EqualTo(writeDesc.Width));
        Assert.That(() => readDesc.Height, Is.EqualTo(writeDesc.Height));
        Assert.That(() => readDesc.Depth, Is.EqualTo(writeDesc.Depth));
        Assert.That(() => readDesc.Format, Is.EqualTo(writeDesc.Format));
    }

    struct TextureDesc : IBinarySerializable<TextureDesc>
    {
        public uint Width { get; set; }
        public uint Height { get; set; }
        public uint Depth { get; set; }
        public PixelFormat Format { get; set; }

        public static TextureDesc Read(ref ReadByteStream stream, TextureDesc existingInstance)
        {
            existingInstance.Width = stream.Read<uint>();
            existingInstance.Height = stream.Read<uint>();
            existingInstance.Depth = stream.Read<uint>();
            existingInstance.Format = stream.ReadEnum<PixelFormat>();

            return existingInstance;
        }

        public static void Write(ref WriteByteStream stream, TextureDesc value)
        {
            stream.Write(value.Width);
            stream.Write(value.Height);
            stream.Write(value.Depth);
            stream.WriteEnum(value.Format);
        }
    }
}
