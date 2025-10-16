// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using XenoAtom.Collections;

namespace Alimer;

public readonly unsafe struct Utf8StringArray : IDisposable
{
    private readonly byte** _data;
    public readonly uint Length;

    public Utf8StringArray(IList<string> strings)
    {
        Length = (uint)strings.Count;
        _data = (byte**)NativeMemory.Alloc((nuint)(strings.Count * sizeof(byte*)));

        for (int i = 0; i < Length; i++)
        {
            ReadOnlySpan<char> source = strings[i];
            int maxLength = Encoding.UTF8.GetMaxByteCount(source.Length);
            Span<byte> bytes = new byte[maxLength + 1];

            int length = Encoding.UTF8.GetBytes(source, bytes);

            uint size = (uint)(bytes.Length + 1) * sizeof(byte);
            _data[i] = (byte*)NativeMemory.Alloc(size);

            fixed (byte* pBytes = bytes)
            {
                NativeMemory.Copy(pBytes, _data[i], size);
            }
        }
    }

    public Utf8StringArray(UnsafeList<Utf8String> strings)
    {
        Length = (uint)strings.Count;
        _data = (byte**)NativeMemory.Alloc((nuint)(strings.Count * sizeof(byte*)));

        for (int i = 0; i < Length; i++)
        {
            ReadOnlySpan<byte> bytes = strings[i].Span;

            uint size = (uint)(bytes.Length + 1) * sizeof(byte);
            _data[i] = (byte*)NativeMemory.Alloc(size);

            fixed (byte* pBytes = bytes)
            {
                NativeMemory.Copy(pBytes, _data[i], size);
            }
        }
    }

    public Utf8StringArray(IList<Utf8String> strings)
    {
        Length = (uint)strings.Count;
        _data = (byte**)NativeMemory.Alloc((nuint)(strings.Count * sizeof(byte*)));

        for (int i = 0; i < Length; i++)
        {
            ReadOnlySpan<byte> bytes = strings[i].Span;

            uint size = (uint)(bytes.Length + 1) * sizeof(byte);
            _data[i] = (byte*)NativeMemory.AllocZeroed(size);

            fixed (byte* pBytes = bytes)
            {
                NativeMemory.Copy(pBytes, _data[i], size);
            }
        }
    }

    public Utf8StringArray(IEnumerable<Utf8String> strings)
    {
        Length = (uint)strings.Count();
        _data = (byte**)NativeMemory.Alloc((nuint)(Length * sizeof(byte*)));

        int index = 0;
        foreach (Utf8String str in strings)
        {
            ReadOnlySpan<byte> bytes = str.Span;

            uint size = (uint)(bytes.Length + 1) * sizeof(byte);
            _data[index] = (byte*)NativeMemory.AllocZeroed(size);

            fixed (byte* pBytes = bytes)
            {
                NativeMemory.Copy(pBytes, _data[index], size);
            }
            index++;
        }
    }

    public void Dispose()
    {
        for (int i = 0; i < Length; i++)
            NativeMemory.Free(_data[i]);

        NativeMemory.Free(_data);
    }

    public override string ToString()
    {
        StringBuilder builder = new("[");

        for (int i = 0; i < Length; i++)
        {
            builder.Append(new string((sbyte*)_data[i]));

            if (i < Length - 1)
                builder.Append(", ");
        }

        builder.Append(']');

        return builder.ToString();
    }

    public static implicit operator byte**(Utf8StringArray pStringArray) => pStringArray._data;
}
