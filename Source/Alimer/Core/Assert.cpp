// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"

using namespace Alimer;

namespace
{
    Assert::FailBehavior DefaultHandler(const char* condition, const std::string& message, const char* file, const uint32_t line)
    {
        std::string printMessage = std::format("{}({}): Assert Failure: ", file, line);

        if (condition != nullptr)
        {
            printMessage += std::format("'{}' ", condition);
        }

        if (!message.empty())
        {
            printMessage += message;
        }

        Log::Critical(printMessage);
        return Assert::FailBehavior::Halt;
    }

    Assert::AssertHandlerFn& GetAssertHandlerInstance()
    {
        static Assert::AssertHandlerFn s_handler = &DefaultHandler;
        return s_handler;
    }
}


Assert::AssertHandlerFn Assert::GetHandler()
{
    return GetAssertHandlerInstance();
}

void Assert::SetHandler(AssertHandlerFn newHandler)
{
    GetAssertHandlerInstance() = newHandler;
}

Assert::FailBehavior Assert::ReportFailure(const char* condition, const char* file, int line, const std::string& message)
{
    return GetAssertHandlerInstance()(condition, message, file, line);
}
