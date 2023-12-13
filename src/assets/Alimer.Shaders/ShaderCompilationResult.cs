// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

namespace Alimer.Shaders;

public abstract class ShaderCompilationResult : IDisposable
{
    protected ShaderCompilationResult() => ErrorMessage = string.Empty;
    protected ShaderCompilationResult(string error) => ErrorMessage = error;

    public string ErrorMessage { get; }
    public bool Failed => !string.IsNullOrWhiteSpace(ErrorMessage);
    public bool Succeeded => !Failed;

    public abstract void Dispose();

    public abstract ReadOnlySpan<byte> GetByteCode();
}
