// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Alimer/Assets/Asset.h"
#include "Alimer/Audio/Types.h"

struct ma_decoder;

namespace Alimer
{
    class AudioClip;
    using AudioClipRef = SharedPtr<AudioClip>;

    class ALIMER_API AudioClip final : public Asset
    {
        ALIMER_OBJECT(AudioClip, Asset);

    public:
        /// Register object factory and properties.
        static void Register();

        /// Destructor
        ~AudioClip() override;

        bool DefineEncoded(const void* data, const void* pData, size_t sizeInBytes);
        bool DefineDecoded(const void* data, uint64_t frameCount, AudioFormat format, uint32_t channels, uint32_t sampleRate);

        bool IsStreamed() const { return _streamed; }

        [[nodiscard]] ma_decoder* GetDecoder() const { return _decoder; }

    protected:
        bool BeginLoad(Stream& source) override;

    private:
        static bool IsWAV(const uint8_t* data, size_t size);
        static bool IsOgg(const uint8_t* data, size_t size);

        ma_decoder* _decoder = nullptr;
        AudioFormat _format = AudioFormat::Float32;
        uint32_t _channels = 2u;
        uint32_t _sampleRate = 0;
        uint64_t _frames = 0;
        bool _streamed = false;
    };
}
