// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using CommunityToolkit.Diagnostics;
using GLTF2;

namespace Alimer.Assets.Graphics;

public static class GltfUtils
{
    const uint GLTFHEADER = 0x46546C67;
    const uint GLTFVERSION2 = 2;
    const uint CHUNKJSON = 0x4E4F534A;
    const uint CHUNKBIN = 0x004E4942;

    public static Gltf2? ParseGltf(Stream stream)
    {
        bool binaryFile = IsBinary(stream);
        if (binaryFile)
        {
            using (BinaryReader binaryReader = new(stream))
            {
                ReadBinaryHeader(binaryReader);

                ReadOnlySpan<byte> data = ReadBinaryChunk(binaryReader, CHUNKJSON);
                return Gltf2.Deserialize(data);
                //return  Encoding.UTF8.GetString(data);
            }
        }
        else
        {
            return Gltf2.Deserialize(stream);
        }
    }

    private static void ReadBinaryHeader(BinaryReader binaryReader)
    {
        uint magic = binaryReader.ReadUInt32();
        if (magic != GLTFHEADER)
        {
            throw new InvalidDataException($"Unexpected magic number: {magic}");
        }

        uint version = binaryReader.ReadUInt32();
        if (version != GLTFVERSION2)
        {
            throw new InvalidDataException($"Unknown version number: {version}");
        }

        uint length = binaryReader.ReadUInt32();
        long fileLength = binaryReader.BaseStream.Length;
        if (length != fileLength)
        {
            throw new InvalidDataException($"The specified length of the file ({length}) is not equal to the actual length of the file ({fileLength}).");
        }
    }

    private static byte[] ReadBinaryChunk(BinaryReader binaryReader, uint format)
    {
        while (true) // keep reading until EndOfFile exception
        {
            uint chunkLength = binaryReader.ReadUInt32();
            if ((chunkLength & 3) != 0)
            {
                throw new InvalidDataException($"The chunk must be padded to 4 bytes: {chunkLength}");
            }

            uint chunkFormat = binaryReader.ReadUInt32();

            var data = binaryReader.ReadBytes((int)chunkLength);

            if (chunkFormat == format) return data;
        }
    }


    public static bool IsBinary(Stream stream)
    {
        Guard.IsNotNull(stream, nameof(stream));
        Guard.IsTrue(stream.CanSeek, nameof(stream), "A seekable stream is required for glTF/GLB format identification");

        long currPos = stream.Position;

        byte a = (byte)stream.ReadByte();
        byte b = (byte)stream.ReadByte();
        byte c = (byte)stream.ReadByte();
        byte d = (byte)stream.ReadByte();

        stream.Position = currPos; // restart read position

        return IsBinaryHeader(a, b, c, d);
    }

    public static bool IsBinaryHeader(byte a, byte b, byte c, byte d)
    {
        uint magic = 0;
        magic |= (uint)a;
        magic |= (uint)b << 8;
        magic |= (uint)c << 16;
        magic |= (uint)d << 24;

        return magic == GLTFHEADER;
    }
}

