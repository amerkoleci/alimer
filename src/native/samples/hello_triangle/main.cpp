// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "alimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <array>
#include <assert.h>
#include <vector>

std::string GetTexturesPath()
{
    return "assets/textures/";
}

std::vector<uint8_t> LoadTexture(const char* fileName)
{
    std::ifstream is(GetTexturesPath() + "/" + fileName, std::ios::binary | std::ios::in | std::ios::ate);
    
    if (is.is_open())
    {
        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);
        char* shaderCode = new char[size];
        is.read(shaderCode, size);
        is.close();

        assert(size > 0);

        std::vector<uint8_t> data(size);
        memcpy(data.data(), shaderCode, size);

        delete[] shaderCode;

        return data;
    }
    else
    {
        std::cerr << "Error: Could not open file \"" << fileName << "\"" << "\n";
        return {};
    }
}

void TestPng()
{
    std::vector<uint8_t> fileData = LoadTexture("10points.png");
    AlimerImage* image = AlimerImage_CreateFromMemory(fileData.data(), fileData.size());
    TextureDimension dimension = AlimerImage_GetDimension(image);
    PixelFormat format = AlimerImage_GetFormat(image);

    assert(dimension == TextureDimension_2D);
    assert(format == PixelFormat_RGBA8Unorm);
    AlimerImage_Destroy(image);
}

void TestHdr()
{
    std::vector<uint8_t> fileData = LoadTexture("environment.hdr");
    AlimerImage* image = AlimerImage_CreateFromMemory(fileData.data(), fileData.size());
    TextureDimension dimension = AlimerImage_GetDimension(image);
    PixelFormat format = AlimerImage_GetFormat(image);

    assert(dimension == TextureDimension_2D);
    assert(format == PixelFormat_RGBA32Float);
    AlimerImage_Destroy(image);
}

int main()
{
    //TestPng();
    //TestHdr();

    Config config{};

    if (!Alimer_Init(&config))
        return 1;

    Alimer_Shutdown();
    return 0;
}
