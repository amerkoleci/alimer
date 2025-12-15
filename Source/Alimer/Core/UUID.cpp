// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "UUID.h"
#include <random>

using namespace Alimer;

namespace
{
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<uint64_t> s_UniformDistribution;
}

UUID::UUID()
    : value(s_UniformDistribution(s_Engine))
{
}

UUID::UUID(uint64_t uuid)
    : value(uuid)
{
}
