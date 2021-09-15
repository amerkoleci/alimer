// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if TODO
#include "Graphics/ShaderCompiler.h"
#include "Graphics/Graphics.h"
#include "IO/FileSystem.h"
#include "Core/Log.h"
#include <spdlog/fmt/fmt.h>

#if defined(_WIN32)
#include <atlbase.h>
#include "PlatformInclude.h"
#include <dxcapi.h>

namespace Alimer::ShaderCompiler
{
    DxcCreateInstanceProc DxcCreateInstance = nullptr;

    std::wstring ShaderProfileName(ShaderStage stage, ShaderModel shaderModel)
    {
        std::wstring shaderProfile;
        switch (stage)
        {
        case ShaderStage::Vertex:
            shaderProfile = L"vs";
            break;
        case ShaderStage::Hull:
            shaderProfile = L"hs";
            break;
        case ShaderStage::Domain:
            shaderProfile = L"ds";
            break;
        case ShaderStage::Geometry:
            shaderProfile = L"gs";
            break;
        case ShaderStage::Pixel:
            shaderProfile = L"ps";
            break;
        case ShaderStage::Compute:
            shaderProfile = L"cs";
            break;

        default:
            ALIMER_UNREACHABLE();
        }

        shaderProfile.push_back(L'_');
        shaderProfile.push_back(L'0' + shaderModel.major);
        shaderProfile.push_back(L'_');
        shaderProfile.push_back(L'0' + shaderModel.minor);

        return shaderProfile;
    }

    class IncludeHandler : public IDxcIncludeHandler
    {
    private:
        CComPtr<IDxcUtils> utils;
        std::string fullPath;

    public:
        IncludeHandler(CComPtr<IDxcUtils> utils_, const std::string& fullPath_)
            : utils(utils_)
            , fullPath(fullPath_)
        {
        }

        HRESULT STDMETHODCALLTYPE LoadSource(
            _In_ LPCWSTR pFilename,
            _COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource) override
        {
            // TODO: pFileName is different when in RELEASE
            auto filePath = Path::Join(fullPath, FileNameAndExtension(StringUtils::ToUtf8(pFilename)));
            if (!File::Exists(filePath))
            {
                return S_FALSE;
            }

            CComPtr<IDxcBlobEncoding> pEncoding;
            HRESULT hr = utils->LoadFile(StringUtils::ToUtf16(filePath).c_str(), DXC_CP_ACP, &pEncoding);
            if (SUCCEEDED(hr))
            {
                *ppIncludeSource = pEncoding.Detach();
            }
            return hr;
        }


        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject) override { return E_NOTIMPL; }
        ULONG STDMETHODCALLTYPE AddRef() override { return E_NOTIMPL; }
        ULONG STDMETHODCALLTYPE Release() override { return E_NOTIMPL; }
    };

    ShaderRef Compile(const std::string& fileName, ShaderBlobType blobType)
    {
        ShaderCompileOptions options{};
        options.source = File::ReadAllText(fileName);
        return Compile(options, blobType);
    }

    ShaderRef Compile(ShaderStage stage, const std::string& fileName, ShaderBlobType blobType)
    {
        ShaderCompileOptions options{};
        options.source = File::ReadAllText(fileName);
        if (stage == ShaderStage::Vertex)
        {
            options.entryPoint = "VSMain";
        }
        else if (stage == ShaderStage::Pixel)
        {
            options.entryPoint = "PSMain";
        }

        options.fileName = fileName;
        options.stage = stage;
        return Compile(options, blobType);
    }

    ShaderRef Compile(const ShaderCompileOptions& options, ShaderBlobType blobType)
    {
        if (DxcCreateInstance == nullptr)
        {
#if defined(_WIN32)
            void* dxcompilerDLL = LoadLibrary("dxcompiler.dll");
            if (dxcompilerDLL == nullptr)
            {
                LOGE("Failed to load dxcompiler.dll");
                return {};
            }

            DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(dxcompilerDLL, "DxcCreateInstance");
            ALIMER_ASSERT(DxcCreateInstance != nullptr);
            LOGD("ShaderCompiler: dxcompiler.dll with success");
#endif
        }

        CComPtr<IDxcUtils> dxcUtils;
        CComPtr<IDxcCompiler3> dxcCompiler;
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)));
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)));

        // Setup arguments 
        std::vector<LPCWSTR> args;

        // Add source file name if present. This will be displayed in error messages
        if (!options.fileName.empty())
        {
            std::wstring wFileName = StringUtils::ToUtf16(options.fileName);
            args.push_back(wFileName.c_str());
        }

        // Entry point
        std::wstring entryPointW = StringUtils::ToUtf16(options.entryPoint);
        args.push_back(L"-E"); args.push_back(entryPointW.c_str());

        // Target
        args.push_back(L"-T");
        auto profile = ShaderProfileName(options.stage, options.shaderModel);
        args.push_back(profile.c_str());

        args.push_back(DXC_ARG_PACK_MATRIX_COLUMN_MAJOR); // Column major
#ifdef _DEBUG
        args.push_back(DXC_ARG_SKIP_OPTIMIZATIONS); // Skip optimizations.
        args.push_back(DXC_ARG_DEBUG); // Enable debug information.
#else
        args.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
#endif

        args.push_back(DXC_ARG_RESOURCES_MAY_ALIAS);
        args.push_back(L"-flegacy-macro-expansion");
        //args.push_back(L"-no-legacy-cbuf-layout");
        //args.push_back(L"-pack-optimized");
        //args.push_back(DXC_ARG_ALL_RESOURCES_BOUND);

        // Defines
        for (auto& define : options.defines)
        {
            std::wstring wDefine = StringUtils::ToUtf16(define);
            args.push_back(L"-D"); args.push_back(wDefine.c_str());
        }

        if (gGraphics().IsInitialized() && gGraphics().GetCaps().features.bindlessDescriptors)
        {
            args.push_back(L"-D"); args.push_back(L"BINDLESS");
        }

        switch (blobType)
        {
        case ShaderBlobType::DXIL:
            args.push_back(L"-D"); args.push_back(L"DXIL");
            break;
        case ShaderBlobType::SPIRV:
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

        std::string fullPath = GetPath(options.fileName);
        if (!fullPath.empty())
        {
            fullPath = Path::Join(Directory::GetCurrent(), fullPath);
            //std::wstring wFullPath = StringUtils::ToUtf16(fullPath);
            //args.push_back(L"-I");
            //args.push_back(wFullPath.c_str());
        }

        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = options.source.c_str();
        sourceBuffer.Size = options.source.size();
        sourceBuffer.Encoding = DXC_CP_ACP;

        IncludeHandler includeHandler(dxcUtils, fullPath);

        CComPtr<IDxcResult> pResults;
        HRESULT hr = dxcCompiler->Compile(
            &sourceBuffer,                          // Source buffer.
            args.data(),							// Array of pointers to arguments.
            static_cast<UINT32>(args.size()),		// Number of arguments.
            &includeHandler,
            IID_PPV_ARGS(&pResults)                  // Compiler output status, buffer, and errors.
        );

        if (FAILED(hr)) {
            return {};
        }

        CComPtr<IDxcBlobUtf8> pErrorsUtf8 = nullptr;
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
        CComPtr<IDxcBlob> pShader = nullptr;
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

        std::vector<uint8_t> bytecode(pShader->GetBufferSize());
        memcpy(bytecode.data(), pShader->GetBufferPointer(), pShader->GetBufferSize());
        return Shader::Create(options.stage, bytecode, options.entryPoint);
}
}

#else

namespace Alimer
{
    std::vector<uint8_t> Compile(const ShaderCompileOptions& options)
    {
        return {};
    }
}

#endif

#endif // TODO
