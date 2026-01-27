// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using Alimer.Graphics;
using CommunityToolkit.Diagnostics;

namespace Alimer.Rendering;

public interface IShaderCompiler
{
    ShaderModule Compile(string fileName, ShaderStages stage, Dictionary<string, string>? defines = default);
}

public sealed class ShaderSystem : IDisposable
{
    private Dictionary<int, ShaderModule> _shaderModules = [];

    public ShaderSystem(GraphicsDevice device)
    {
        Device = device;
        Backend = device.Backend;
    }

    public GraphicsDevice Device { get; }
    public GraphicsBackend Backend { get; }
    public List<string> Paths { get; } = [];
    public IShaderCompiler? Compiler { get; set; }

    public void Dispose()
    {
        foreach(ShaderModule module in _shaderModules.Values)
        {
            module.Dispose();
        }
        _shaderModules.Clear();
    }

    public void AddPath(string path)
    {
        Guard.IsTrue(Directory.Exists(path), nameof(path), $"The specified shader path '{path}' does not exist.");

        Paths.Add(path);
    }

    public ShaderModule GetShaderModule(string name, ShaderStages stage, Dictionary<string, string>? defines = default)
    {
        int key = HashCode.Combine(name, stage);
        if (_shaderModules.TryGetValue(key, out ShaderModule? module))
        {
            return module;
        }

        Guard.IsNotNull(Compiler);
        module = Compiler.Compile(name, stage, defines);
        if (module is null)
        {
            throw new InvalidOperationException($"Shader compiler failed to compile shader '{name}' with stage '{stage}'.");
        }

        _shaderModules.Add(key, module);
        return module;
    }

}
