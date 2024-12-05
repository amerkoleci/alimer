// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer_internal.h"
#include "alimer.h"

void alimerGetVersion(uint32_t* major, uint32_t* minor, uint32_t* patch)
{
    if (major) *major = ALIMER_VERSION_MAJOR;
    if (minor) *minor = ALIMER_VERSION_MINOR;
    if (patch) *patch = ALIMER_VERSION_PATCH;
}

/* Memory */
// TODO: Add custom memory allocation
// TODO: Add tracy profile (TracyCAlloc)
void* alimerCalloc(size_t count, size_t size)
{
    if (count && size)
    {
        void* block;

        if (count > SIZE_MAX / size)
        {
            alimerLogError(LogCategory_System, "Allocation size overflow");
            return NULL;
        }

        block = malloc(count * size);
        if (block)
        {
            return memset(block, 0, count * size);
        }
        else
        {
            alimerLogFatal(LogCategory_System, "Out of memory");
            return nullptr;
        }
    }

    return nullptr;
}

void* alimerMalloc(size_t size)
{
    return alimerCalloc(1, size);
}

void* alimerRealloc(void* old, size_t size)
{
    void* data = realloc(old, size);
    if (!data) abort();
    //alimerProfileAlloc(data, size);
    return data;
}

void alimerFree(void* data)
{
    //alimerProfileFree(data);
    free(data);
}

Blob* alimerBlobCreate(void* data, size_t size, const char* name)
{
    Blob* blob = ALIMER_ALLOC(Blob);
    blob->ref = 1;
    blob->data = data;
    blob->size = size;
    if (name)
        blob->name = _alimer_strdup(name);
    return blob;
}

void alimerBlobDestroy(Blob* blob)
{
    alimerFree(blob->data);
    alimerFree(blob->name);
    alimerFree(blob);
}

char* _alimer_strdup(const char* source)
{
    const size_t length = strlen(source);
    char* result = (char*)alimerCalloc(length + 1, 1);
    memcpy(result, source, length + 1);
    return result;
}


#if defined(_WIN32)
/// Returns a wide string version of the specified UTF-8 string
WCHAR* Win32_CreateWideStringFromUTF8(const char* source)
{
    WCHAR* target;
    int count;

    count = MultiByteToWideChar(CP_UTF8, 0, source, -1, NULL, 0);
    if (!count)
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string from UTF-8");
        return nullptr;
    }

    target = ALIMER_ALLOCN(WCHAR, count);

    if (!MultiByteToWideChar(CP_UTF8, 0, source, -1, target, count))
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string from UTF-8");
        alimerFree(target);
        return nullptr;
    }

    return target;
}

/// Returns a UTF-8 string version of the specified wide string
char* Win32_CreateUTF8FromWideString(const WCHAR* source)
{
    char* target;
    int size;

    size = WideCharToMultiByte(CP_UTF8, 0, source, -1, NULL, 0, NULL, NULL);
    if (!size)
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string to UTF-8");
        return nullptr;
    }

    target = ALIMER_ALLOCN(char, size);

    if (!WideCharToMultiByte(CP_UTF8, 0, source, -1, target, size, NULL, NULL))
    {
        alimerLogError(LogCategory_System, "Win32: Failed to convert string to UTF-8");
        alimerFree(target);
        return nullptr;
    }

    return target;
}

#endif
