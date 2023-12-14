using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using Hebron.Runtime;

namespace StbImageSharp;

#if !STBSHARP_INTERNAL
	public
#else
internal
#endif
unsafe class ImageResult : IDisposable
{
    public int Width { get; set; }
    public int Height { get; set; }
    public ColorComponents SourceComp { get; set; }
    public ColorComponents Comp { get; set; }
    public byte* DataPtr { get; set; }
    public int DataLength { get; set; }
    public Span<byte> Data => new(DataPtr, DataLength);

    ~ImageResult()
    {
        Dispose(false);
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    private void Dispose(bool disposing)
    {
        CRuntime.free(DataPtr);
    }

    internal static unsafe ImageResult FromResult(byte* result, int width, int height, ColorComponents comp,
        ColorComponents req_comp)
    {
        if (result == null)
            throw new InvalidOperationException(StbImage.stbi__g_failure_reason);

        var image = new ImageResult
        {
            Width = width,
            Height = height,
            SourceComp = comp,
            Comp = req_comp == ColorComponents.Default ? comp : req_comp,
            DataPtr = result,
        };
        image.DataLength = width * height * (int)image.Comp;

        return image;
    }

    public static unsafe ImageResult FromStream(Stream stream,
        ColorComponents requiredComponents = ColorComponents.Default)
    {
        int x, y, comp;

        var context = new StbImage.stbi__context(stream);

        var result = StbImage.stbi__load_and_postprocess_8bit(context, &x, &y, &comp, (int)requiredComponents);

        return FromResult(result, x, y, (ColorComponents)comp, requiredComponents);
    }

    public static ImageResult FromMemory(byte[] data, ColorComponents requiredComponents = ColorComponents.Default)
    {
        using (var stream = new MemoryStream(data))
        {
            return FromStream(stream, requiredComponents);
        }
    }

    public static IEnumerable<AnimatedFrameResult> AnimatedGifFramesFromStream(Stream stream,
        ColorComponents requiredComponents = ColorComponents.Default)
    {
        return new AnimatedGifEnumerable(stream, requiredComponents);
    }
}
