using System;
using System.IO;
using System.Runtime.InteropServices;
using Hebron.Runtime;

namespace StbImageSharp;

#if !STBSHARP_INTERNAL
	public
#else
internal
#endif
unsafe class ImageResultFloat : IDisposable
{
    public int Width { get; set; }
    public int Height { get; set; }
    public ColorComponents SourceComp { get; set; }
    public ColorComponents Comp { get; set; }

    public float* DataPtr { get; set; }
    public int DataLength { get; set; }
    public Span<float> Data => new(DataPtr, DataLength);

    private void Dispose(bool disposing)
    {
        CRuntime.free(DataPtr);
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    ~ImageResultFloat()
    {
        Dispose(false);
    }

    internal static unsafe ImageResultFloat FromResult(float* result, int width, int height, ColorComponents comp,
        ColorComponents req_comp)
    {
        if (result == null)
            throw new InvalidOperationException(StbImage.stbi__g_failure_reason);

        ImageResultFloat image = new()
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

    public static unsafe ImageResultFloat FromStream(Stream stream, ColorComponents requiredComponents = ColorComponents.Default)
    {
        int x, y, comp;

        var context = new StbImage.stbi__context(stream);

        var result = StbImage.stbi__loadf_main(context, &x, &y, &comp, (int)requiredComponents);

        return FromResult(result, x, y, (ColorComponents)comp, requiredComponents);
    }

    public static ImageResultFloat FromMemory(byte[] data, ColorComponents requiredComponents = ColorComponents.Default)
    {
        using (var stream = new MemoryStream(data))
        {
            return FromStream(stream, requiredComponents);
        }
    }
}
