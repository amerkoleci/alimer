// Copyright © Amer Koleci.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "RHI.h"

#if defined(ALIMER_RHI_VULKAN)
#include "RHI_Vulkan.h"
#endif

#include <cstdio>
#include <cstdarg>

namespace RHI
{
    /* Loggin */
    static LogFunction s_LogFunc = nullptr;

    void SetLogFunction(LogFunction function)
    {
        s_LogFunc = function;
    }

    void LogInfo(const char* format, ...)
    {
        if (!s_LogFunc)
            return;

        char msg[kMaxLogMessageSize];
        va_list args;
        va_start(args, format);
        vsnprintf(msg, sizeof(msg), format, args);
        va_end(args);
        s_LogFunc(LogLevel::Info, msg);
    }

    void LogDebug(const char* format, ...)
    {
        if (!s_LogFunc)
            return;

        char msg[kMaxLogMessageSize];
        va_list args;
        va_start(args, format);
        vsnprintf(msg, sizeof(msg), format, args);
        va_end(args);
        s_LogFunc(LogLevel::Debug, msg);
    }

    void LogWarn(const char* format, ...)
    {
        if (!s_LogFunc)
            return;

        char msg[kMaxLogMessageSize];
        va_list args;
        va_start(args, format);
        vsnprintf(msg, sizeof(msg), format, args);
        va_end(args);
        s_LogFunc(LogLevel::Warn, msg);
    }

    void LogError(const char* format, ...)
    {
        if (!s_LogFunc)
            return;

        char msg[kMaxLogMessageSize];
        va_list args;
        va_start(args, format);
        vsnprintf(msg, sizeof(msg), format, args);
        va_end(args);
        s_LogFunc(LogLevel::Error, msg);
    }

    /* Helper methods */
    const char* GetVendorName(uint32_t vendorId)
    {
        switch (vendorId)
        {
            case KnownVendorId_AMD:
                return "AMD";
            case KnownVendorId_ImgTec:
                return "IMAGINATION";
            case KnownVendorId_Nvidia:
                return "Nvidia";
            case KnownVendorId_ARM:
                return "ARM";
            case KnownVendorId_Qualcomm:
                return "Qualcom";
            case KnownVendorId_Intel:
                return "Intel";
            default:
                return "Unknown";
        }
    }

    /* Implementation */
    DeviceHandle CreateDevice(GraphicsAPI api, ValidationMode validationMode)
    {
#if defined(ALIMER_RHI_VULKAN)
        return DeviceHandle::Create(new VulkanDevice(validationMode));
#endif
    }
}
