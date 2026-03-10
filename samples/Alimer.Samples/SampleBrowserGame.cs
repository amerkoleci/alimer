// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Numerics;
using System.Text;
using System.Text.Json;
using Alimer.Engine;
using Alimer.Graphics;
using Alimer.Physics;
using Alimer.Rendering;
using Alimer.Samples.Graphics;
using Alimer.Serialization;

namespace Alimer.Samples;

// https://github.com/dotnet/runtime/tree/main/src/tests/nativeaot
public sealed class SampleBrowserGame : Game
{
    private SampleBase _runningSample = null!;

    public SampleBrowserGame(GraphicsBackend preferredGraphicsBackend = GraphicsBackend.Default)
        : base(preferredGraphicsBackend)
    {
        GameSystems.Add(new ImGuiSystem(Services));
    }

    protected override void Initialize()
    {
        base.Initialize();

#if TODO
        {
            using MemoryStream memoryStream = new();
            using (var serializer = Serializer.CreateJson(memoryStream, new JsonWriterOptions() { Indented = true }))
            {
                serializer.BeginObject("Inner");
                serializer.Serialize("Value", (byte)123);
                serializer.EndObject();
            }
            var json = Encoding.UTF8.GetString(memoryStream.ToArray());
            memoryStream.Position = 0;
            using (var deserializer = Deserializer.CreateJson(memoryStream))
            {
                deserializer.BeginObject("Inner");
                //var value = deserializer.Deserialize<byte>("Value");
                deserializer.EndObject();
                //Console.WriteLine($"Deserialized value: {value}");
            }
        } 
#endif


        // Setup shader system (until we have a proper asset pipeline)
        string shadersPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Shaders");
        ShaderSystem.AddPath(shadersPath);
        ShaderSystem.Compiler = new AssetShaderCompiler(ShaderSystem);

        //string texturesPath = Path.Combine(AppContext.BaseDirectory, "Assets", "Textures");
        //Image image = Image.FromFile(Path.Combine(texturesPath, "10points.png"));

        //_runningSample = new HelloWindowSample(Services, MainWindow);
        //_runningSample = new DrawTriangleSample(Services, MainWindow);
        //_runningSample = new DrawIndexedQuadSample(Services, MainWindow);
        //_runningSample = new DrawCubeSample(Services, MainWindow);
        //_runningSample = new DrawTexturedCubeSample(Services, MainWindow);
        // _runningSample = new DrawTexturedFromFileCubeSample(Services, MainWindow);
        //_runningSample = new DrawMeshSample(Services, MainWindow);

        var testEntity = new Entity();
        CameraComponent camera = testEntity.AddComponent<CameraComponent>();
        camera.Entity!.Transform.Position = new Vector3(0.0f, 0.0f, 5.0f);

        RigidBodyComponent rigidBody = testEntity.AddComponent<RigidBodyComponent>();
        //var testJson = testEntity.Serialize();
        //var json = testJson.ToJsonString();
        //var loaded = Entity.Deserialize(json);

        // Engine samples (scene)
        //_runningSample = new SceneCubeSample(Services);
        _runningSample = new ScenePBRRendererSample(Services);

        WriteByteStream writer = new(4);
        writer.Write(1);
        writer.Write(true);
        writer.Write(Vector3.One);
        writer.Write(Matrix4x4.Identity);
        writer.Write("HELLO WORLD");
        writer.WriteEnum(LightType.Point);
        var testObject = new MyByteWrite();
        testObject.FieldA = 123;
        writer.Write(testObject);

        ReadOnlySpan<byte> span = writer.WrittenSpan;

        // Read now
        ReadByteStream readStream = new(span);
        int readInt = readStream.Read<int>();
        bool readBool = readStream.Read<bool>();
        Vector3 readVector3 = readStream.Read<Vector3>();
        Matrix4x4 readMatrix4x4 = readStream.Read<Matrix4x4>();
        string readString = readStream.ReadString()!;
        LightType readEnum = readStream.ReadEnum<LightType>();
        MyByteWrite readMyByteWrite = readStream.ReadByteSerializable<MyByteWrite>();

        MainWindow.Title = $"{_runningSample.Name} - {GraphicsDevice.Backend}";
    }

    class MyByteWrite : IByteSerializable
    {
        public int FieldA { get; set; }

        public static object ReadObject(ref ReadByteStream stream)
        {
            MyByteWrite myByteWrite = new MyByteWrite();
            myByteWrite.FieldA = stream.Read<int>();
            return myByteWrite;
        }

        public static void WriteObject(ref WriteByteStream stream, object value)
        {
            MyByteWrite myByteWrite = (MyByteWrite)value;
            stream.Write(myByteWrite.FieldA);
        }
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            _runningSample.Dispose();
        }

        base.Dispose(disposing);
    }

    protected override void Update(GameTime time)
    {
        base.Update(time);

        _runningSample.Update(time);
    }

    protected override void Draw(CommandBuffer renderContext, Texture outputTexture, GameTime time)
    {
        _runningSample.Draw(renderContext, outputTexture);

        base.Draw(renderContext, outputTexture, time);
    }

    public static void Main()
    {
        GraphicsBackend preferredGraphicsBackend = GraphicsBackend.Default;

#if !WINDOWS
        preferredGraphicsBackend = GraphicsBackend.Vulkan;
        //preferredGraphicsBackend = GraphicsBackend.Metal;
        //preferredGraphicsBackend = GraphicsBackend.Null;
#endif

        using SampleBrowserGame game = new(preferredGraphicsBackend);
        game.Run();
    }
}
