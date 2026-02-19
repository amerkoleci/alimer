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

    const string EMBEDDEDOCTETSTREAM = "data:application/octet-stream;base64,";
    const string EMBEDDEDGLTFBUFFER = "data:application/gltf-buffer;base64,";

    const string EMBEDDEDPNG = "data:image/png;base64,";
    const string EMBEDDEDJPEG = "data:image/jpeg;base64,";

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
            }
        }
        else
        {
            return Gltf2.Deserialize(stream);
        }
    }

    /// <summary>
    /// Loads the binary buffer chunk of a glb file
    /// </summary>
    /// <param name="filePath">Source file path to a glb model</param>
    /// <returns>Byte array of the buffer</returns>
    public static byte[] LoadBinaryBuffer(string filePath)
    {
        using Stream stream = File.OpenRead(filePath);
        return LoadBinaryBuffer(stream);
    }

    /// <summary>
    /// Reads the binary buffer chunk of a glb stream
    /// </summary>
    /// <param name="stream">Readable stream to a glb model</param>
    /// <returns>Byte array of the buffer</returns>
    public static byte[] LoadBinaryBuffer(Stream stream)
    {
        using BinaryReader binaryReader = new(stream);
        ReadBinaryHeader(binaryReader);

        return ReadBinaryChunk(binaryReader, CHUNKBIN);
    }

    /// <summary>
    /// Gets a binary buffer referenced by a specific <code>Schema.Buffer</code>
    /// </summary>
    /// <param name="model">The <code>Schema.Gltf</code> model containing the <code>Schema.Buffer</code></param>
    /// <param name="bufferIndex">The index of the buffer</param>
    /// <param name="gltfFilePath">Source file path used to load the model</param>
    /// <returns>Byte array of the buffer</returns>
    public static byte[] LoadBinaryBuffer(this Gltf2 model, int bufferIndex, string gltfFilePath)
    {
        return LoadBinaryBuffer(model, bufferIndex, GetExternalFileSolver(gltfFilePath));
    }

    /// <summary>
    /// Gets a binary buffer referenced by a specific <code>Schema.Buffer</code>
    /// </summary>
    /// <param name="model">The <code>Schema.Gltf</code> model containing the <code>Schema.Buffer</code></param>
    /// <param name="bufferIndex">The index of the buffer</param>
    /// <param name="externalReferenceSolver">An user provided lambda function to resolve external assets</param>
    /// <returns>Byte array of the buffer</returns>
    /// <remarks>
    /// Binary buffers can be stored in three different ways:
    /// - As stand alone files.
    /// - As a binary chunk within a glb file.
    /// - Encoded to Base64 within the JSON.
    /// 
    /// The external reference solver funcion is called when the buffer is stored in an external file,
    /// or when the buffer is in the glb binary chunk, in which case, the Argument of the function will be Null.
    /// 
    /// The Lambda function must return the byte array of the requested file or buffer.
    /// </remarks>
    public static byte[] LoadBinaryBuffer(this Gltf2 model, int bufferIndex, Func<string, byte[]> externalReferenceSolver)
    {
        Gltf2.Buffer buffer = model.Buffers[bufferIndex];

        var bufferData = LoadBinaryBufferUnchecked(buffer, externalReferenceSolver);

        // As per https://github.com/KhronosGroup/glTF/issues/1026
        // Due to buffer padding, buffer length can be equal or larger than expected length by only 3 bytes
        if (bufferData.Length < buffer.ByteLength || (bufferData.Length - buffer.ByteLength) > 3)
        {
            throw new InvalidDataException($"The buffer length is {bufferData.Length}, expected {buffer.ByteLength}");
        }

        return bufferData;
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

            byte[] data = binaryReader.ReadBytes((int)chunkLength);

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

    /// <summary>
    /// Creates an External File Solver for a given gltf file path, so we can resolve references to associated files
    /// </summary>
    /// <param name="gltfFilePath">ource file path to a gltf/glb model</param>
    /// <returns>Lambda funcion to resolve dependencies</returns>
    private static Func<string, byte[]> GetExternalFileSolver(string gltfFilePath)
    {
        return asset =>
        {
            if (string.IsNullOrEmpty(asset))
                return LoadBinaryBuffer(gltfFilePath);

            string bufferFilePath = Path.Combine(Path.GetDirectoryName(gltfFilePath)!, asset);
            return File.ReadAllBytes(bufferFilePath);
        };
    }

    private static byte[]? TryLoadBase64BinaryBufferUnchecked(Gltf2.Buffer buffer, string prefix)
    {
        if (buffer.Uri == null || !buffer.Uri.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
            return null;

        string content = buffer.Uri.Substring(prefix.Length);
        return Convert.FromBase64String(content);
    }

    private static byte[]? LoadBinaryBufferUnchecked(Gltf2.Buffer buffer, Func<string, byte[]> externalReferenceSolver)
    {
        return TryLoadBase64BinaryBufferUnchecked(buffer, EMBEDDEDGLTFBUFFER)
            ?? TryLoadBase64BinaryBufferUnchecked(buffer, EMBEDDEDOCTETSTREAM)
            ?? externalReferenceSolver(buffer.Uri);
    }
}

