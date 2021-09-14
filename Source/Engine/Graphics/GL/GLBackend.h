// Copyright © Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "Graphics/GraphicsDefs.h"
#include "PlatformInclude.h"
#if ALIMER_PLATFORM_EMSCRIPTEN
#   include <GLES3/gl3.h>
#   include <GLES2/gl2ext.h>
#   include <GL/gl.h>
#   include <GL/glext.h>
#else
#   include <glad/glad.h>
#endif

namespace Alimer
{
    struct GL_Buffer final
    {
        GLuint id = 0;

        ~GL_Buffer();
    };
}

