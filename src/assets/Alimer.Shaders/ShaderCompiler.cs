// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Alimer.Graphics;
using Vortice.SPIRV.Reflect;
using Win32;
using Win32.Graphics.Direct3D.Dxc;
using static Win32.Apis;
using static Win32.Graphics.Direct3D.Dxc.Apis;
using static Vortice.SPIRV.Reflect.SPIRVReflectApi;
using Win32.Graphics.Direct3D12;
using static Vortice.SpirvCross.SpirvCrossApi;
using Vortice.SpirvCross;
using Vortice.SPIRV;

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
        ReadOnlySpan<char> source,
        in ShaderCompilationOptions options)
    {
        using ComPtr<IDxcResult> results = default;

        string shaderTarget = GetShaderTarget(options.ShaderStage, options.ShaderModel);
        List<string> arguments = new(32)
        {
            options.SourceFileName,

            // -T
            "-T",
            shaderTarget,

            // -E
            "-E",
            options.EntryPoint
        };

        // Defines
        arguments.Add("-D");
        arguments.Add("COMPILER_DXC");

        if (options.ShaderFormat == ShaderFormat.SPIRV)
        {
            arguments.Add("-D");
            arguments.Add("SPIRV");

            arguments.Add("-D");
            arguments.Add("VULKAN");
        }

        if (options.Defines.Count > 0)
        {
            foreach (KeyValuePair<string, string> definePair in options.Defines)
            {
                arguments.Add("-D");
                arguments.Add($"{definePair.Key}={definePair.Value}");
            }
        }

        // Include directories
        if (options.IncludeDirs.Count > 0)
        {
            foreach (string dir in options.IncludeDirs)
            {
                arguments.Add("-I");
                arguments.Add($"\"{dir}\"");
            }
        }

        if (options.SkipOptimizations)
        {
            arguments.Add(DXC_ARG_SKIP_OPTIMIZATIONS);
        }
        else
        {
            if (options.OptimizationLevel < 4)
            {
                arguments.Add($"-O{options.OptimizationLevel}");
            }
            else
            {
                throw new InvalidOperationException("Invalid optimization level.");
            }
        }

        if (options.SkipValidation)
        {
            arguments.Add(DXC_ARG_SKIP_VALIDATION);
        }

        if (options.ShaderModel.Major >= 6 && options.ShaderModel.Minor >= 2)
        {
            arguments.Add("-enable-16bit-types");
        }

        if (options.WarningsAreErrors)
            arguments.Add(DXC_ARG_WARNINGS_ARE_ERRORS);

        if (options.EnableStrictness)
            arguments.Add(DXC_ARG_ENABLE_STRICTNESS);

        if (options.Debug)
            arguments.Add(DXC_ARG_DEBUG);

        if (options.AllResourcesBound)
            arguments.Add(DXC_ARG_ALL_RESOURCES_BOUND);

        if (options.PackMatrixRowMajor)
            arguments.Add(DXC_ARG_PACK_MATRIX_ROW_MAJOR);

        if (options.PackMatrixColumnMajor)
            arguments.Add(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR);

        if (options.ShaderFormat == ShaderFormat.SPIRV)
        {
            arguments.Add("-spirv");
            arguments.Add($"-fspv-target-env=vulkan{options.SpvTargetEnvMajor}.{options.SpvTargetEnvMinor}");

            arguments.Add("-fvk-use-dx-layout");
            arguments.Add("-fvk-use-dx-position-w");

            for (int space = 0; space < options.ShiftSpaceCount; space++)
            {
                arguments.Add("-fvk-b-shift");
                arguments.Add($"{options.SpirvBShift}");
                arguments.Add($"{space}");

                arguments.Add("-fvk-t-shift");
                arguments.Add($"{options.SpirvTShift}");
                arguments.Add($"{space}");

                arguments.Add("-fvk-u-shift");
                arguments.Add($"{options.SpirvUShift}");
                arguments.Add($"{space}");

                arguments.Add("-fvk-s-shift");
                arguments.Add($"{options.SpirvSShift}");
                arguments.Add($"{space}");
            }
        }
        else
        {
            // -Qstrip_reflect
            if (options.StripReflection)
                arguments.Add("-Qstrip_reflect");

            if (options.StripDebug && !options.Debug)
                arguments.Add("-Qstrip_debug");
        }

        HResult hr = Compile(source, arguments.ToArray(), __uuidof<IDxcResult>(), results.GetVoidAddressOf());

        if (hr.Failure)
        {
            return new DxcShaderCompilationResult($"Compile failed with HRESULT {hr}");
        }

        //
        // Print errors if present.
        //
        using ComPtr<IDxcBlobUtf8> errors = default;
        results.Get()->GetOutput(DXC_OUT_ERRORS,
            __uuidof<IDxcBlobUtf8>(),
            errors.GetVoidAddressOf(),
            null
            );
        // Note that d3dcompiler would return null if no errors or warnings are present.
        // IDxcCompiler3::Compile will always return an error buffer, but its length
        // will be zero if there are no warnings or errors.
        if (errors.Get() is not null && errors.Get()->GetStringLength() != 0)
        {
            string warningAndErrors = StringUtilities.GetString(errors.Get()->GetStringPointer(), (int)errors.Get()->GetStringLength())!;
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
        results.Get()->GetOutput(DXC_OUT_OBJECT,
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
        results.Get()->GetOutput(DXC_OUT_PDB,
            __uuidof<IDxcBlob>(),
            pPDB.GetVoidAddressOf(),
            pPDBName.GetAddressOf());
        if (pPDB.Get() is null)
        {
        }

        // Print hash.
        using ComPtr<IDxcBlob> hash = default;
        results.Get()->GetOutput(DXC_OUT_SHADER_HASH,
            __uuidof<IDxcBlob>(),
            hash.GetVoidAddressOf(),
            null);
        if (hash.Get() is not null)
        {
        }

        if (options.ShaderFormat == ShaderFormat.DXIL)
        {
            using ComPtr<IDxcBlob> reflectionData = default;
            results.Get()->GetOutput(DXC_OUT_REFLECTION, __uuidof<IDxcBlob>(), reflectionData.GetVoidAddressOf(), null);
            if (reflectionData.Get() is not null)
            {
                // Create reflection interface.
                DxcBuffer reflectionDataBuffer = new()
                {
                    Encoding = DXC_CP_ACP,
                    Ptr = reflectionData.Get()->GetBufferPointer(),
                    Size = reflectionData.Get()->GetBufferSize()
                };

                using ComPtr<ID3D12ShaderReflection> reflection = default;
                _dxcUtils.Get()->CreateReflection(&reflectionDataBuffer, __uuidof<ID3D12ShaderReflection>(), reflection.GetVoidAddressOf());

                ShaderDescription description;
                ThrowIfFailed(reflection.Get()->GetDesc(&description));
#if TODO

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
#endif
            }
        }
        else
        {
            ReadOnlySpan<byte> bytecode = new (byteCode.Get()->GetBufferPointer(), (int)byteCode.Get()->GetBufferSize());

            {
                spvc_context_create(out spvc_context context).CheckResult();
                spvc_context_parse_spirv(context, bytecode, out spvc_parsed_ir parsedIr).CheckResult();
                spvc_context_create_compiler(context, Backend.HLSL, parsedIr, CaptureMode.TakeOwnership, out spvc_compiler compiler).CheckResult();

                spvc_compiler_create_compiler_options(compiler, out spvc_compiler_options compileOptions).CheckResult();
                spvc_compiler_options_set_uint(compileOptions, CompilerOption.HLSLShaderModel, 50);
                //spvc_compiler_set_entry_point(compileOptions, "", );
                spvc_compiler_install_compiler_options(compiler, compileOptions);

                spvc_compiler_create_shader_resources(compiler, out spvc_resources resources);
                FlatDescriptorBindingsForType(compiler, resources, ResourceType.UniformBuffer);
                FlatDescriptorBindingsForType(compiler, resources, ResourceType.SampledImage);
                FlatDescriptorBindingsForType(compiler, resources, ResourceType.SeparateSamplers);
                FlatDescriptorBindingsForType(compiler, resources, ResourceType.SeparateImage);

                if (spvc_compiler_compile(compiler, out string? hlsl) != Result.Success)
                {
                    string errorMessage = spvc_context_get_last_error_string(context)!;
                    throw new Exception($"Failed to compile SPIRV shader: {errorMessage}");
                }

                spvc_context_release_allocations(context);
                spvc_context_destroy(context);
            }

            // Use SpirvReflect for reflection
            SpvReflectShaderModule module = default;
            spvReflectCreateShaderModule(bytecode, &module).CheckResult();

            uint binding_count = 0;
            spvReflectEnumerateDescriptorBindings(&module, &binding_count, null).CheckResult();

            SpvReflectDescriptorBinding** bindings = stackalloc SpvReflectDescriptorBinding*[(int)binding_count];
            spvReflectEnumerateDescriptorBindings(&module, &binding_count, bindings).CheckResult();

            for (int i = 0; i < binding_count; i++)
            {
                ref SpvReflectDescriptorBinding binding = ref *bindings[i];
            }

            spvReflectDestroyShaderModule(&module);
        }

        return new DxcShaderCompilationResult(byteCode);
    }

    private static unsafe void FlatDescriptorBindingsForType(
        in spvc_compiler compiler,
        in spvc_resources resources,
        ResourceType type)
    {
        // D3D11/OpenGL has flat namespace so remap bindings

        spvc_reflected_resource* resourceList;
        spvc_resources_get_resource_list_for_type(resources, type, out resourceList, out nuint resourceSize).CheckResult();

        for (uint i = 0; i < (uint)resourceSize; i++)
        {
            uint id = resourceList[i].id;

            uint currentSet = spvc_compiler_get_decoration(compiler, resourceList[i].id, SpvDecoration.DescriptorSet);
            uint currentBinding = spvc_compiler_get_decoration(compiler, resourceList[i].id, SpvDecoration.Binding);

            spvc_compiler_unset_decoration(compiler, id, SpvDecoration.DescriptorSet);
            //spvc_compiler_set_decoration(compiler, id, SpvDecoration.Binding, newBinding);
        }
    }

    private HResult Compile(ReadOnlySpan<char> source, string[] arguments, Guid* riid, void** ppResult)
    {
        using ComPtr<IDxcBlobEncoding> dxcBlobEncoding = default;

        fixed (char* pSource = source)
        {
            ThrowIfFailed(_dxcUtils.Get()->CreateBlobFromPinned(
                pSource,
                (uint)source.Length * 2,
                DXC_CP_UTF16,
                dxcBlobEncoding.GetAddressOf())
                );
        }

        DxcBuffer buffer = new()
        {
            Ptr = dxcBlobEncoding.Get()->GetBufferPointer(),
            Size = dxcBlobEncoding.Get()->GetBufferSize(),
            Encoding = DXC_CP_UTF16,
        };

        nint* pArguments = StringsToUtf16(arguments);

        try
        {
            HResult hr = _dxcCompiler.Get()->Compile(
                &buffer,
                (char**)pArguments,
                (uint)arguments.Length,
                _dxcDefaultIncludeHandler.Get(),
                riid,
                ppResult
                );

            return hr;
        }
        finally
        {
            NativeMemory.Free(pArguments);
        }
    }

    public static string GetShaderTarget(ShaderStages stage, in DxcShaderModel shaderModel)
    {
        string model = $"{shaderModel.Major}_{shaderModel.Minor}";
        switch (stage)
        {
            case ShaderStages.Vertex:
                return $"vs_{model}";

            case ShaderStages.Hull:
                return $"hs_{model}";

            case ShaderStages.Domain:
                return $"ds_{model}";

            case ShaderStages.Geometry:
                return $"gs_{model}";

            case ShaderStages.Fragment:
                return $"ps_{model}";

            case ShaderStages.Compute:
                return $"cs_{model}";

            case ShaderStages.Mesh:
                return $"ms_{model}";

            case ShaderStages.Amplification:
                return $"as_{model}";

            default:
            case ShaderStages.Library:
                return $"lib_{model}";
        }
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static void* Allocate(nuint size)
    {
        void* result = NativeMemory.Alloc(size);

        if (result == null)
        {
            throw new OutOfMemoryException($"The allocation of '{size}' bytes failed");
        }

        return result;
    }

    private static nint* StringsToUtf16(string[] values)
    {
        if (values == null || values.Length == 0)
            return null;

        // Allocate unmanaged memory for string pointers.
        nint* stringHandlesPtr = (nint*)Allocate((nuint)(sizeof(nint) * values.Length));

        // Store the pointer to the string.
        for (int i = 0; i < values.Length; i++)
        {
            stringHandlesPtr[i] = Marshal.StringToHGlobalUni(values[i]);
        }

        return stringHandlesPtr;
    }
}
