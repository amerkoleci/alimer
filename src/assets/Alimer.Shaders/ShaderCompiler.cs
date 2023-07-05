// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using Win32;
using Win32.Graphics.Direct3D;
using Win32.Graphics.Direct3D.Dxc;
using Win32.Graphics.Direct3D12;
using static Win32.Apis;
using static Win32.Graphics.Direct3D.Dxc.Apis;

namespace Alimer.Shaders;

public sealed unsafe partial class ShaderCompiler
{
    /// <summary>
    /// The thread local <see cref="ShaderCompiler"/> instance.
    /// This is necessary because the DXC library is strictly single-threaded.
    /// </summary>
    [ThreadStatic]
    private static ShaderCompiler? s_instance;

    /// <summary>
    /// The <see cref="IDxcCompiler"/> instance to use to create the bytecode for HLSL sources.
    /// </summary>
    private readonly ComPtr<IDxcCompiler3> _dxcCompiler;

    /// <summary>
    /// The <see cref="IDxcUtils"/> instance to use to work with <see cref="_dxcCompiler"/>.
    /// </summary>
    private readonly ComPtr<IDxcUtils> _dxcUtils;

    /// <summary>
    /// The <see cref="IDxcIncludeHandler"/> instance used to compile shaders with <see cref="_dxcCompiler"/>.
    /// </summary>
    private readonly ComPtr<IDxcIncludeHandler> _dxcDefaultIncludeHandler;

    /// <summary>
    /// Creates a new <see cref="ShaderCompiler"/> instance.
    /// </summary>
    private ShaderCompiler()
    {
        using ComPtr<IDxcUtils> dxcUtils = default;
        using ComPtr<IDxcCompiler3> dxcCompiler = default;
        using ComPtr<IDxcIncludeHandler> dxcIncludeHandler = default;

        ThrowIfFailed(DxcCreateInstance(
           (Guid*)Unsafe.AsPointer(ref Unsafe.AsRef(in CLSID_DxcLibrary)),
            __uuidof<IDxcUtils>(),
            dxcUtils.GetVoidAddressOf())
            );
        ThrowIfFailed(DxcCreateInstance(
            (Guid*)Unsafe.AsPointer(ref Unsafe.AsRef(in CLSID_DxcCompiler)),
            __uuidof<IDxcCompiler3>(),
            dxcCompiler.GetVoidAddressOf())
            );

        ThrowIfFailed(
            dxcUtils.Get()->CreateDefaultIncludeHandler(dxcIncludeHandler.GetAddressOf())
            );

        _dxcCompiler = dxcCompiler.Move();
        _dxcUtils = dxcUtils.Move();
        _dxcDefaultIncludeHandler = dxcIncludeHandler.Move();
    }

    /// <summary>
    /// Destroys the current <see cref="ShaderCompiler"/> instance.
    /// </summary>
    ~ShaderCompiler()
    {
        _dxcCompiler.Dispose();
        _dxcUtils.Dispose();
        _dxcDefaultIncludeHandler.Dispose();
    }

    /// <summary>
    /// Gets a <see cref="ShaderCompiler"/> instance to use.
    /// </summary>
    public static ShaderCompiler Instance => s_instance ??= new();

    public ShaderCompilationResult Compile(
        ShaderFormat format,
        ReadOnlySpan<char> source,
        ReadOnlySpan<char> entryPoint, ReadOnlySpan<char> target)
    {
        using ComPtr<IDxcBlobEncoding> dxcBlobEncoding = default;
        using ComPtr<IDxcResult> results = default;

        // Get the encoded blob from the source code
        fixed (char* pSource = source)
        {
            ThrowIfFailed(_dxcUtils.Get()->CreateBlobFromPinned(
                pSource,
                (uint)source.Length * 2,
                DxcCp.Utf16,
                dxcBlobEncoding.GetAddressOf())
                );
        }

        DxcBuffer buffer = new()
        {
            Ptr = dxcBlobEncoding.Get()->GetBufferPointer(),
            Size = dxcBlobEncoding.Get()->GetBufferSize(),
            Encoding = DxcCp.Utf16,
        };

        fixed (char* shaderName = "")
        fixed (char* pEntryPointName = "-E")
        fixed (char* pEntryPoint = entryPoint)
        fixed (char* pShaderProfileName = "-T")
        fixed (char* pShaderProfile = target)
        fixed (char* pIncludeParameter = "-I")
        fixed (char* pDefineParameter = "-D")
        fixed (char* pStripReflect = "-Qstrip_reflect")
        fixed (char* pSpirvArg = "-spirv")
        fixed (char* pSpvTargetEnvArg = "-fspv-target-env=vulkan1.2")
        fixed (char* pVkUseDxLayoutArg = "-fvk-use-dx-layout")
        fixed (char* pVkUseDxDxPositionArg = "-fvk-use-dx-position-w")
        fixed (char* pSpirvDefine = "SPIRV")
        {
            uint argCount = 0;
            char** arguments = stackalloc char*[32];
            arguments[argCount++] = shaderName;
            // -E
            arguments[argCount++] = pEntryPointName;
            arguments[argCount++] = pEntryPoint;

            // -T
            arguments[argCount++] = pShaderProfileName;
            arguments[argCount++] = pShaderProfile;

            if (format == ShaderFormat.SPIRV)
            {
                arguments[argCount++] = pDefineParameter;
                arguments[argCount++] = pSpirvDefine;

                arguments[argCount++] = pSpirvArg;
                arguments[argCount++] = pSpvTargetEnvArg;

                arguments[argCount++] = pVkUseDxLayoutArg;
                arguments[argCount++] = pVkUseDxDxPositionArg;
            }
            else
            {
                // -Qstrip_reflect
                arguments[argCount++] = pStripReflect;
            }

            HResult hr = _dxcCompiler.Get()->Compile(
                &buffer,
                (ushort**)arguments,
                argCount,
                _dxcDefaultIncludeHandler.Get(),
                __uuidof<IDxcResult>(),
                results.GetVoidAddressOf()
                );

            if (hr.Failure)
            {
                return new DxcShaderCompilationResult($"Compile failed with HRESULT {hr}");
            }

            //
            // Print errors if present.
            //
            using ComPtr<IDxcBlobUtf8> errors = default;
            results.Get()->GetOutput(DxcOutKind.Errors,
                __uuidof<IDxcBlobUtf8>(),
                errors.GetVoidAddressOf(),
                null
                );
            // Note that d3dcompiler would return null if no errors or warnings are present.
            // IDxcCompiler3::Compile will always return an error buffer, but its length
            // will be zero if there are no warnings or errors.
            if (errors.Get() is not null && errors.Get()->GetStringLength() != 0)
            {
                string warningAndErrors = new(errors.Get()->GetStringPointer());
                //Log.Warn"Warnings and Errors:\n%S\n", pErrors->GetStringPointer());
            }

            // Quit if the compilation failed.
            HResult hrStatus;
            results.Get()->GetStatus(&hrStatus);
            if (hrStatus.Failure)
            {
                return new DxcShaderCompilationResult($"Compile failed with HRESULT {hrStatus}");
            }

            using ComPtr<IDxcBlob> byteCode = default;
            using ComPtr<IDxcBlobUtf16> pShaderName = default;
            results.Get()->GetOutput(DxcOutKind.Object,
                __uuidof<IDxcBlob>(),
                byteCode.GetVoidAddressOf(),
                pShaderName.GetAddressOf()
                );
            if (byteCode.Get() is null)
            {
                return new DxcShaderCompilationResult("The compiled shader is invalid");
            }

            // Save pdb.
            using ComPtr<IDxcBlob> pPDB = default;
            using ComPtr<IDxcBlobUtf16> pPDBName = default;
            results.Get()->GetOutput(DxcOutKind.Pdb,
                __uuidof<IDxcBlob>(),
                pPDB.GetVoidAddressOf(),
                pPDBName.GetAddressOf());
            if (pPDB.Get() is null)
            {
            }

            // Print hash.
            using ComPtr<IDxcBlob> hash = default;
            results.Get()->GetOutput(DxcOutKind.ShaderHash,
                __uuidof<IDxcBlob>(),
                hash.GetVoidAddressOf(),
                null);
            if (hash.Get() is not null)
            {
            }

            if (format == ShaderFormat.DXIL)
            {
                using ComPtr<IDxcBlob> reflectionData = default;
                results.Get()->GetOutput(DxcOutKind.Reflection, __uuidof<IDxcBlob>(), reflectionData.GetVoidAddressOf(), null);
                if (reflectionData.Get() is not null)
                {
                    // Create reflection interface.
                    var ReflectionData = new DxcBuffer
                    {
                        Encoding = DxcCp.Acp,
                        Ptr = reflectionData.Get()->GetBufferPointer(),
                        Size = reflectionData.Get()->GetBufferSize()
                    };

                    using ComPtr<ID3D12ShaderReflection> reflection = default;
                    _dxcUtils.Get()->CreateReflection(&ReflectionData, __uuidof<ID3D12ShaderReflection>(), reflection.GetVoidAddressOf());

                    ShaderDescription description;
                    ThrowIfFailed(reflection.Get()->GetDesc(&description));

                    // Iterate on all Constant buffers used by this shader
                    // Build all ParameterBuffers
                    for (uint i = 0; i < description.ConstantBuffers; i++)
                    {
                        ID3D12ShaderReflectionConstantBuffer* reflectConstantBuffer = reflection.Get()->GetConstantBufferByIndex(i);
                        ShaderBufferDescription reflectConstantBufferDescription = default;
                        ThrowIfFailed(reflectConstantBuffer->GetDesc(&reflectConstantBufferDescription));

                        // Skip non pure constant-buffers and texture buffers
                        if (reflectConstantBufferDescription.Type != ConstantBufferType.ConstantBuffer
                            && reflectConstantBufferDescription.Type != ConstantBufferType.TextureBuffer)
                        {
                            continue;
                        }

                        string name = new(reflectConstantBufferDescription.Name);

                        // Create the buffer
                        //var parameterBuffer = new EffectData.ConstantBuffer()
                        //{
                        //    Name = reflectConstantBufferDescription.Name,
                        //    Size = reflectConstantBufferDescription.Size,
                        //    Parameters = new List<EffectData.ValueTypeParameter>(),
                        //};
                        //shader.ConstantBuffers.Add(parameterBuffer);
                        //
                        //// Iterate on each variable declared inside this buffer
                        //for (int j = 0; j < reflectConstantBufferDescription.VariableCount; j++)
                        //{
                        //    var variableBuffer = reflectConstantBuffer.GetVariable(j);
                        //
                        //    // Build constant buffer parameter
                        //    var parameter = BuildConstantBufferParameter(variableBuffer);
                        //
                        //    // Add this parameter to the ConstantBuffer
                        //    parameterBuffer.Parameters.Add(parameter);
                        //}
                    }

                    for (uint i = 0; i < description.BoundResources; ++i)
                    {
                        ShaderInputBindDescription shaderInputBindDesc = default;
                        ThrowIfFailed(reflection.Get()->GetResourceBindingDesc(i, &shaderInputBindDesc));
                        string name = new(shaderInputBindDesc.Name);

                    }
                }
            }

            return new DxcShaderCompilationResult(byteCode);
        }
    }
}
