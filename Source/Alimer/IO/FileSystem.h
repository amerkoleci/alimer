// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/IO/Stream.h"
#include "Alimer/Core/Containers.h"

namespace Alimer
{
    /// File stream class
    class ALIMER_API FileStream : public Stream
    {
    public:
        /// Constructor.
        FileStream() = default;
        /// Constructor and open a file.
        FileStream(std::string_view fileName, FileMode mode = FileMode::Read);
        /// Destructor. Close the file if open.
        ~FileStream() override;

        /// Read bytes from the file. Return number of bytes actually read.
        size_t Read(void* buffer, size_t length) override;
        /// Set position in bytes from the beginning of the file.
        size_t Seek(size_t position) override;
        /// Write bytes to the file. Return number of bytes actually written.
        size_t Write(const void* buffer, size_t length) override;
        /// Return whether read operations are allowed.
        bool CanRead() const override;
        /// Return whether write operations are allowed.
        bool CanWrite() const override;
        size_t GetPosition() const override;
        size_t GetSize() const override;

        /// Open a file. Return true on success.
        bool Open(std::string_view fileName, FileMode mode = FileMode::Read);
        /// Close the file.
        void Close();
        /// Flush any buffered output to the file.
        void Flush();

        /// Return the open mode.
        FileMode GetMode() const { return mode; }
        /// Return whether is open.
        bool IsOpen() const;
        /// Return the file handle.
        void* GetHandle() const { return handle; }

        using Stream::Read;
        using Stream::Write;

    private:
        /// Open mode.
        FileMode mode = FileMode::Read;

        /// File handle.
        void* handle = nullptr;
    };

    namespace Path
    {
        ALIMER_API std::string Normalize(std::string_view path);
        ALIMER_API std::string Combine(std::string_view path1, std::string_view path2);
        ALIMER_API std::string Combine(std::string_view path1, std::string_view path2, std::string_view path3);

        /// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
        ALIMER_API void SplitPath(std::string_view fullPath, std::string& pathName, std::string& fileName, std::string& extension, bool lowercaseExtension = true);

        /// Return the path from a full path.
        ALIMER_API std::string GetFullPath(std::string_view fullPath);

        /// Return the filename from a full path.
        ALIMER_API std::string GetFileName(const std::string& fullPath);
        /// Return the extension from a full path, converted to lowercase by default.
        ALIMER_API std::string GetExtension(std::string_view fullPath, bool lowercaseExtension = true);
    }

    namespace File
    {
        ALIMER_API bool Exists(StringView path);

        ALIMER_API std::string ReadAllText(StringView path);
        ALIMER_API Vector<uint8_t> ReadAllBytes(StringView path);
        ALIMER_API bool ReadAllBytes(StringView path, Vector<uint8_t>& data);
        ALIMER_API bool WriteAllBytes(StringView path, const uint8_t* data, size_t size);
        ALIMER_API size_t GetFileSizeInBytes(StringView path);
    }

    namespace Directory
    {
        ALIMER_API bool Exists(StringView path);
    }
}
