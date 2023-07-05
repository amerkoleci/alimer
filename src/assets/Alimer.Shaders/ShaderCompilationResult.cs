// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Shaders;

public abstract class ShaderCompilationResult : IDisposable
{
    protected ShaderCompilationResult() => Error = string.Empty;
    protected ShaderCompilationResult(string error) => Error = error;


    public string Error { get; }
    public bool Failed => !string.IsNullOrWhiteSpace(Error);
    public bool Succeeded => !Failed;

    public abstract void Dispose();

    public abstract ReadOnlySpan<byte> GetByteCode();
}
