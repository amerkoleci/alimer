// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Graphics/Shader.h"
#include "Graphics/Graphics.h"
#include "Core/Log.h"

#if defined(_WIN32)
#   include "Core/StringUtils.h"
#   include "PlatformInclude.h"
#   include <wrl/client.h>
#   include <dxcapi.h>
#endif

namespace Alimer
{
#if defined(_WIN32)
    IDxcUtils* dxcUtils = nullptr;

    namespace
    {
        using Microsoft::WRL::ComPtr;

        DxcCreateInstanceProc DxcCreateInstance = nullptr;

        struct ShaderModel
        {
            uint8_t major = 6;
            uint8_t minor = 0;
        };

        inline std::wstring ShaderProfileName(ShaderStages stage, ShaderModel shaderModel)
        {
            std::wstring shaderProfile;
            switch (stage)
            {
            case ShaderStages::Vertex:
                shaderProfile = L"vs";
                break;
            case ShaderStages::Hull:
                shaderProfile = L"hs";
                break;
            case ShaderStages::Domain:
                shaderProfile = L"ds";
                break;
            case ShaderStages::Geometry:
                shaderProfile = L"gs";
                break;
            case ShaderStages::Pixel:
                shaderProfile = L"ps";
                break;
            case ShaderStages::Compute:
                shaderProfile = L"cs";
                break;
            case ShaderStages::Amplification:
                shaderProfile = L"as";
                break;
            case ShaderStages::Mesh:
                shaderProfile = L"ms";
                break;

            default:
                shaderProfile = L"lib";
                break;
            }

            shaderProfile.push_back(L'_');
            shaderProfile.push_back(L'0' + shaderModel.major);
            shaderProfile.push_back(L'_');
            shaderProfile.push_back(L'0' + shaderModel.minor);

            return shaderProfile;
        }

        static IDxcCompiler3* dxcCompiler = nullptr;

        inline ShaderRef CompileDXIL(ShaderStages stage, const std::string& source, const std::string& entryPoint)
        {
            if (DxcCreateInstance == nullptr)
            {
                HMODULE dxcompilerDLL = LoadLibraryW(L"dxcompiler.dll");
                if (dxcompilerDLL == nullptr)
                {
                    LOGE("Failed to load dxcompiler.dll");
                    return {};
                }

                DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(dxcompilerDLL, "DxcCreateInstance");
                ALIMER_ASSERT(DxcCreateInstance != nullptr);
                LOGD("ShaderCompiler: dxcompiler.dll with success");
            }

            if (!dxcUtils)
            {
                ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));
            }

            if (!dxcCompiler)
            {
                ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));
            }

            ComPtr<IDxcIncludeHandler> includeHandler;
            ThrowIfFailed(dxcUtils->CreateDefaultIncludeHandler(&includeHandler));

            ShaderModel shaderModel{ 6, 5 };

            // Setup arguments 
            std::vector<LPCWSTR> args;

            // Add source file name if present. This will be displayed in error messages
            //if (!options.fileName.empty())
            //{
            //    std::wstring wFileName = ToUtf16(options.fileName);
            //    args.push_back(wFileName.c_str());
            //}

            // Entry point
            std::wstring entryPointW = ToUtf16(entryPoint);
            args.push_back(L"-E"); args.push_back(entryPointW.c_str());

            // Target
            args.push_back(L"-T");
            auto profile = ShaderProfileName(stage, shaderModel);
            args.push_back(profile.c_str());

            //args.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR); // Column major
#ifdef _DEBUG
            args.push_back(L"-Od"); // Skip optimizations.
            args.push_back(DXC_ARG_DEBUG); // Enable debug information.
#else
            args.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

            args.push_back(L"-res_may_alias");
            args.push_back(L"-flegacy-macro-expansion");
            //args.push_back(L"-no-legacy-cbuf-layout");
            //args.push_back(L"-pack-optimized");
            //args.push_back(DXC_ARG_ALL_RESOURCES_BOUND);

            // Defines
            //for (auto& define : options.defines)
            //{
            //    std::wstring wDefine = ToUtf16(define);
            //    args.push_back(L"-D"); args.push_back(wDefine.c_str());
            //}

            switch (gGraphics().GetShaderFormat())
            {
                case ShaderFormat::DXIL:
                    args.push_back(L"-D"); args.push_back(L"DXIL");
                    break;
                case ShaderFormat::SPIRV:
                    args.push_back(L"-D"); args.push_back(L"SPIRV");
                    args.push_back(L"-spirv");
                    args.push_back(L"-fspv-target-env=vulkan1.2");
                    args.push_back(L"-fvk-use-dx-layout");
                    args.push_back(L"-fvk-use-dx-position-w");
            
                    //args.push_back(L"-fvk-b-shift"); args.push_back(L"0"); args.push_back(L"0");
                    //args.push_back(L"-fvk-t-shift"); args.push_back(L"1000"); args.push_back(L"0");
                    //args.push_back(L"-fvk-u-shift"); args.push_back(L"2000"); args.push_back(L"0");
                    //args.push_back(L"-fvk-s-shift"); args.push_back(L"3000"); args.push_back(L"0");
                    break;
            }

            //std::string fullPath = GetPath(options.fileName);
            //if (!fullPath.empty())
            //{
            //    fullPath = Path::Join(Directory::GetCurrent(), fullPath);
            //    //std::wstring wFullPath = StringUtils::ToUtf16(fullPath);
            //    //args.push_back(L"-I");
            //    //args.push_back(wFullPath.c_str());
            //}

            DxcBuffer sourceBuffer;
            sourceBuffer.Ptr = source.c_str();
            sourceBuffer.Size = source.size();
            sourceBuffer.Encoding = DXC_CP_ACP;

            //IncludeHandler includeHandler(dxcUtils, fullPath);

            ComPtr<IDxcResult> pResults;
            HRESULT hr = dxcCompiler->Compile(
                &sourceBuffer,                          // Source buffer.
                args.data(),							// Array of pointers to arguments.
                static_cast<UINT32>(args.size()),		// Number of arguments.
                includeHandler.Get(),
                IID_PPV_ARGS(&pResults)                  // Compiler output status, buffer, and errors.
            );

            if (FAILED(hr)) {
                return {};
            }

            ComPtr<IDxcBlobUtf8> pErrorsUtf8 = nullptr;
            hr = pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrorsUtf8), nullptr);
            ALIMER_ASSERT(SUCCEEDED(hr));
            if (pErrorsUtf8 != nullptr && pErrorsUtf8->GetStringLength() > 0)
            {
                OutputDebugStringA(pErrorsUtf8->GetStringPointer());
                std::string errorMessage = pErrorsUtf8->GetStringPointer();
                LOGE("Failed to compiler shader, errors: {}", errorMessage);
            }

            HRESULT hrStatus;
            hr = pResults->GetStatus(&hrStatus);
            ALIMER_ASSERT(SUCCEEDED(hr));
            if (FAILED(hrStatus)) {
                return {};
            }

            // Save shader binary.
            ComPtr<IDxcBlob> pShader = nullptr;
            //CComPtr<IDxcBlobUtf16> pShaderName = nullptr;
            hr = pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), nullptr/* &pShaderName*/);
            ALIMER_ASSERT(SUCCEEDED(hr));

#if TODO
            if (pShader != nullptr)
            {
                FILE* fp = NULL;

                _wfopen_s(&fp, pShaderName->GetStringPointer(), L"wb");
                fwrite(pShader->GetBufferPointer(), pShader->GetBufferSize(), 1, fp);
                fclose(fp);
            }
#endif

            return Shader::Create(stage, pShader->GetBufferPointer(), pShader->GetBufferSize(), entryPoint);
        }
    }
#endif

	Shader::Shader(ShaderStages stage, const std::string& entryPoint_)
        : stage{ stage }
		, entryPoint(entryPoint_)
	{

	}

    ShaderRef Shader::Create(ShaderStages stage, const std::string& source, const std::string& entryPoint)
    {
        ALIMER_ASSERT(gGraphics().IsInitialized());

#if defined(_WIN32)
        return CompileDXIL(stage, source, entryPoint);
#else
        return nullptr;
#endif
    }

	ShaderRef Shader::Create(ShaderStages stage, const void* byteCode, size_t byteCodeLength, const std::string& entryPoint)
	{
        ALIMER_ASSERT(byteCode != nullptr);
		ALIMER_ASSERT(byteCodeLength > 0);
		ALIMER_ASSERT(gGraphics().IsInitialized());

		return gGraphics().CreateShader(stage, byteCode, byteCodeLength, entryPoint);
	}
}
