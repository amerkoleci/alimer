// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/IO/FileSystem.h"
#include "Alimer/Core/Assert.h"
#include "Alimer/Core/Log.h"
#include <cstdio>

#if defined(_WIN32)
#include "Alimer/Platform/Win32/WindowsPlatform.h"
#else
static const char* openMode[] =
{
    "rb",
    "wb",
    "r+b",
    "w+b"
};
#endif

#include <filesystem>

using namespace Alimer;

/* FileStream */
FileStream::FileStream(std::string_view fileName, FileMode mode_)
{
    Open(fileName, mode_);
}

FileStream::~FileStream()
{
    Close();
}

bool FileStream::Open(std::string_view fileName, FileMode mode_)
{
    Close();

    if (fileName.empty())
        return false;

#ifdef _WIN32
    CREATEFILE2_EXTENDED_PARAMETERS param = {};
    param.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
    param.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    param.dwSecurityQosFlags = SECURITY_ANONYMOUS;

    auto wideFileName = ToUtf16(fileName);

    if (mode_ == FileMode::Read)
    {
        if (!File::Exists(fileName))
            return false;

        // Open the file
        handle = CreateFile2(wideFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &param);
    }
    else
    {
        // If the exists, delete it
        if (File::Exists(fileName))
        {
            if (DeleteFileW(wideFileName.c_str()) != TRUE)
            {
                LOGF("{}", GetWin32ErrorString(GetLastError()));
            }
        }

        // Create the file
        handle = CreateFile2(wideFileName.c_str(), GENERIC_WRITE, 0, CREATE_NEW, &param);
        //handle = CreateFile(wideFileName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    if (handle == INVALID_HANDLE_VALUE)
    {
        LOGE("Win32: Failed to access file: {} - {}", fileName, GetWin32ErrorString(GetLastError()));
        return false;
    }
#else
    handle = fopen(Path::Normalize(fileName).c_str(), openMode[(uint32_t)mode_]);
    // If file did not exist in readwrite mode, retry with write-update mode
    if (mode == FileMode::ReadWrite && !handle)
    {
        handle = fopen(Path::Normalize(fileName).c_str(), openMode[uint32_t(mode) + 1]);
    }

    if (!handle)
        return false;
#endif

    _name = fileName;
    mode = mode_;

    return true;
}


void FileStream::Close()
{
#ifndef _WIN32
    if (handle != nullptr)
    {
        fclose((FILE*)handle);
        handle = nullptr;
    }
#else
    if (handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(handle);
        handle = INVALID_HANDLE_VALUE;
    }
#endif
}

size_t FileStream::Read(void* buffer, size_t length)
{
    ALIMER_VERIFY(CanRead());

#ifndef _WIN32
    return fread(buffer, length, 1, (FILE*)handle);
#else
    static const DWORD read_step = 65536;

    size_t read = 0;

    while (read < length)
    {
        DWORD to_read = read_step;
        if (to_read > length - read)
            to_read = (DWORD)(length - read);

        DWORD moved = 0;
        if (ReadFile(handle, (uint8_t*)buffer + read, to_read, &moved, NULL))
            read += moved;

        if (moved < to_read)
            break;
    }

    return read;
#endif
}

size_t FileStream::Seek(size_t position)
{
    if (handle == nullptr)
        return 0;

#ifndef _WIN32
    fseek((FILE*)handle, (long)position, SEEK_SET);
    size_t newPosition = ftell((FILE*)handle);
    return newPosition;
#else
    LARGE_INTEGER move{};
    move.QuadPart = position;

    LARGE_INTEGER result;
    if (SetFilePointerEx(handle, move, &result, FILE_BEGIN))
    {
        return result.QuadPart;
    }

    return 0;
#endif
}


size_t FileStream::Write(const void* buffer, size_t length)
{
    if (!length)
        return 0;

    ALIMER_VERIFY(CanWrite());

#ifndef _WIN32
    return fwrite(buffer, length, 1, (FILE*)handle);
#else

    static const DWORD write_step = 65536;

    size_t written = 0;

    while (written < length)
    {
        DWORD to_write = write_step;
        if (to_write > length - written)
            to_write = (DWORD)(length - written);

        DWORD moved = 0;
        if (WriteFile(handle, (const uint8_t*)buffer + written, to_write, &moved, NULL))
            written += moved;

        if (moved < to_write)
            break;
    }

    return written;
#endif
}

size_t FileStream::GetPosition() const
{
#ifndef _WIN32
    size_t position = ftell((FILE*)handle);
    return position;
#else
    LARGE_INTEGER move;
    LARGE_INTEGER result;

    move.QuadPart = 0;
    result.QuadPart = 0;

    SetFilePointerEx(handle, move, &result, FILE_CURRENT);
    return result.QuadPart;
#endif
}

size_t FileStream::GetSize() const
{
#ifndef _WIN32
    fseek((FILE*)handle, 0, SEEK_END);
    size_t length = ftell((FILE*)handle);
    fseek((FILE*)handle, 0, SEEK_SET);
    return length;
#else
    LARGE_INTEGER result;
    GetFileSizeEx(handle, &result);
    return result.QuadPart;
#endif
}

void FileStream::Flush()
{
#ifndef _WIN32
    if (handle)
    {
        fflush((FILE*)handle);
    }
#else
    if (handle != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(handle);
    }
#endif
}

bool FileStream::CanRead() const
{
    return handle != nullptr && (mode != FileMode::Write);
}

bool FileStream::CanWrite() const
{
    return handle != nullptr && (mode != FileMode::Read);
}

bool FileStream::IsOpen() const
{
    return handle != nullptr;
}

/* Path*/
std::string GetInternalPath(std::string_view pathName)
{
    return StringUtils::ReplaceAll(pathName, "\\", "/");
}

std::string Path::Normalize(std::string_view path)
{
    std::string normalized;

    auto len = path.length();
    for (size_t n = 0; n < len; n++)
    {
        // normalize slashes
        if (path[n] == '\\' || path[n] == '/')
        {
            if (normalized.length() == 0 || normalized[normalized.length() - 1] != '/')
                normalized.append("/");
        }
        // move up a directory
        else if (path[n] == '.' && n < len - 1 && path[n + 1] == '.')
        {
            // search backwards for last /
            bool could_move_up = false;
            if (normalized.length() > 0)
            {
                for (auto k = normalized.length() - 1; k > 0; k--)
                    if (normalized[k - 1] == '/')
                    {
                        normalized = normalized.substr(0, k - 1);
                        could_move_up = true;
                        break;
                    }
            }

            if (!could_move_up)
                normalized.append(".");
            else
                n++;
        }
        else
            normalized += path[n];
    }

    return normalized;
}

std::string Path::Combine(std::string_view path1, std::string_view path2)
{
    std::string normalizedPath1 = Normalize(path1);
    std::string normalizedPath2 = Normalize(path2);

    return Normalize(normalizedPath1.append("/").append(normalizedPath2));
}

std::string Path::Combine(std::string_view path1, std::string_view path2, std::string_view path3)
{
    return Path::Combine(path1, Path::Combine(path2, path3));
}

void Path::SplitPath(std::string_view fullPath, std::string& pathName, std::string& fileName, std::string& extension, bool lowercaseExtension)
{
    std::string fullPathCopy = GetInternalPath(fullPath);

    std::string::size_type extPos = fullPathCopy.find_last_of('.');
    std::string::size_type pathPos = fullPathCopy.find_last_of('/');

    if (extPos != std::string::npos &&
        (pathPos == std::string::npos || extPos > pathPos))
    {
        extension = fullPathCopy.substr(extPos);
        if (lowercaseExtension)
            extension = ToLower(extension);
        fullPathCopy = fullPathCopy.substr(0, extPos);
    }
    else
        extension.clear();

    pathPos = fullPathCopy.find_last_of('/');
    if (pathPos != std::string::npos)
    {
        fileName = fullPathCopy.substr(pathPos + 1);
        pathName = fullPathCopy.substr(0, pathPos + 1);
    }
    else
    {
        fileName = fullPathCopy;
        pathName.clear();
    }
}

std::string Path::GetFullPath(std::string_view fullPath)
{
    std::string path, file, extension;
    SplitPath(fullPath, path, file, extension);
    return path;
}

std::string Path::GetFileName(const std::string& fullPath)
{
    std::string path, file, extension;
    SplitPath(fullPath, path, file, extension);
    return file;
}

std::string Path::GetExtension(std::string_view fullPath, bool lowercaseExtension)
{
    std::string path, file, extension;
    SplitPath(fullPath, path, file, extension, lowercaseExtension);
    return extension;
}

/* File */
bool File::Exists(StringView path)
{
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

std::string File::ReadAllText(StringView path)
{
    if (!File::Exists(path))
        return {};

    FileStream stream(path, FileMode::Read);
    std::string result = stream.ReadString();
    return result;
}

Vector<uint8_t> File::ReadAllBytes(StringView path)
{
    if (!File::Exists(path))
        return {};

    FileStream stream(path, FileMode::Read);
    return stream.ReadBytes();
}

bool File::ReadAllBytes(StringView path, Vector<uint8_t>& data)
{
    if (!File::Exists(path))
        return false;

    FileStream stream(path, FileMode::Read);
    stream.ReadBytes(data);
    return true;
}

bool File::WriteAllBytes(StringView path, const uint8_t* data, size_t size)
{
    if (size <= 0)
    {
        return false;
    }

    FileStream stream(path, FileMode::Write);
    stream.Write(data, size);
    return false;
}

size_t File::GetFileSizeInBytes(StringView path)
{
    if (!File::Exists(path))
        return 0;

    FileStream stream(path, FileMode::Read);
    return stream.GetSize();
}

/* Directory */
bool Directory::Exists(StringView path)
{
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}
