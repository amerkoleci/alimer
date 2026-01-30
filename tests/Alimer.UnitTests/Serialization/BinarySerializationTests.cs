// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using NUnit.Framework;

namespace Alimer.Serialization;

[TestFixture(TestOf = typeof(BinarySerializer))]
public class BinarySerializationTests
{
    [Theory]
    public void TestDefault()
    {
        byte testByte = byte.MaxValue;
        sbyte testSByte = sbyte.MinValue;
        bool testBool = true;
        short testShort = short.MinValue;
        ushort testUShort = ushort.MaxValue;
        int testInt = int.MinValue;
        uint testUInt = uint.MaxValue;
        //long testInt64 = long.MinValue;
        //ulong testUInt64 = ulong.MaxValue;
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
        //serializer.Serialize(ref testInt64);
        //serializer.Serialize(ref testUInt64);
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
        //serializer.Serialize(ref testInt64);
        //serializer.Serialize(ref testUInt64);
        serializer.Serialize(ref testString!);

        // Compare
        Assert.That(() => testByte, Is.EqualTo(byte.MaxValue));
        Assert.That(() => testSByte, Is.EqualTo(sbyte.MinValue));
        Assert.That(() => testBool, Is.True);
        Assert.That(() => testShort, Is.EqualTo(short.MinValue));
        Assert.That(() => testUShort, Is.EqualTo(ushort.MaxValue));
        Assert.That(() => testInt, Is.EqualTo(int.MinValue));
        Assert.That(() => testUInt, Is.EqualTo(uint.MaxValue));

        //Assert.Equal(long.MinValue, testInt64);
        //Assert.Equal(ulong.MaxValue, testUInt64);
        Assert.That(() => testString, Is.EqualTo("HELLO WORLD"));
    }

}
