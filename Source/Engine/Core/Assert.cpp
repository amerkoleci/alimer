// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Core/Assert.h"
#include "Core/Log.h"
#include <cstdio>

namespace Alimer::Assert
{
    namespace
    {
        FailBehavior DefaultHandler(const char* condition, const std::string& message, const char* file, const int line)
        {
            const uint64_t BufferSize = 2048;
            char buffer[BufferSize];
            snprintf(buffer, BufferSize, "%s(%d): Assert Failure: ", file, line);

            if (condition != NULL)
                snprintf(buffer, BufferSize, "%s'%s' ", buffer, condition);

            if (!message.empty())
            {
                snprintf(buffer, BufferSize, "%s%s", buffer, message.c_str());
            }

            snprintf(buffer, BufferSize, "%s\n", buffer);

            LOGE("{}", buffer);

            return FailBehavior::Halt;
        }

        AssertFn& GetAssertHandlerInstance()
        {
            static AssertFn s_handler = &DefaultHandler;
            return s_handler;
        }
    }

    AssertFn GetHandler()
    {
        return GetAssertHandlerInstance();
    }

    void SetHandler(AssertFn newHandler)
    {
        GetAssertHandlerInstance() = newHandler;
    }

    FailBehavior ReportFailure(const char* condition, const char* file, int line, const std::string& message)
    {
        return GetAssertHandlerInstance()(condition, message, file, line);
    }
}
