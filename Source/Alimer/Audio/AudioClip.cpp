// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "Alimer/Core/Log.h"
#include "Alimer/IO/MemoryStream.h"
#include "Alimer/Audio/AudioClip.h"

ALIMER_DISABLE_WARNINGS()
//#define STB_VORBIS_HEADER_ONLY
//#define STB_VORBIS_NO_STDIO
//#include <stb_vorbis.c>
#include <miniaudio.h>
ALIMER_ENABLE_WARNINGS()

using namespace Alimer;

namespace
{
    constexpr AudioFormat FromMiniAudio(ma_format value)
    {
        switch (value)
        {
            case ma_format_s16: return AudioFormat::Int16;
            case ma_format_s32: return AudioFormat::Int32;
            case ma_format_f32: return AudioFormat::Float32;

            default:
                ALIMER_UNREACHABLE();
        }
    }
}

void AudioClip::Register()
{
    RegisterFactory<AudioClip>();
}

AudioClip::~AudioClip()
{
    if (_decoder != nullptr)
    {
        ma_decoder_uninit(_decoder);
        ma_free(_decoder, nullptr);
    }
}

bool AudioClip::BeginLoad(Stream& source)
{
    auto name = source.GetName();
    //size_t dataSize = source.GetSize();
    //std::unique_ptr<uint8_t> buffer(new uint8_t[dataSize]);
    //source.Read(buffer.get(), dataSize);

    if (_decoder != nullptr)
    {
        ma_decoder_uninit(_decoder);
    }
    else
    {
        _decoder = (ma_decoder*)ma_malloc(sizeof(ma_decoder), nullptr);
    }

    ma_result result = ma_decoder_init_file(name.c_str(), nullptr, _decoder);
    //ma_result result = ma_decoder_init_memory(buffer.get(), dataSize, nullptr, _decoder);
    if (result != MA_SUCCESS)
        return false;

    ma_format format;
    result = ma_decoder_get_data_format(_decoder, &format, &_channels, &_sampleRate, nullptr, 0);
    if (result != MA_SUCCESS)
        return false;

    ma_uint64 frames = 0;
    result = ma_decoder_get_length_in_pcm_frames(_decoder, &frames);
    if (result != MA_SUCCESS)
        return false;

    _format = FromMiniAudio(format);
    _frames = static_cast<size_t>(frames);

#if 0
    if (IsWAV(buffer.get(), dataSize))
    {

    }
    else if (IsOgg(buffer.get(), dataSize))
    {
        _streamed = true;
    }
    else
#endif // 0

    {
        _streamed = true;
    }

    return true;
}

bool AudioClip::IsWAV(const uint8_t* data, size_t size)
{
    if (size < 64)
        return false;

    return memcmp(data, "RIFF", 4) == 0;
}

bool AudioClip::IsOgg(const uint8_t* data, size_t size)
{
    if (size < 4)
        return false;

    return memcmp(data, "OggS", 4) == 0;
}
