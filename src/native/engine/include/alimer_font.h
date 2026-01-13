// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ALIMER_FONT_H_
#define ALIMER_FONT_H_ 1

#include "alimer.h"

/* Forward */
typedef struct Font Font;

ALIMER_API Font* alimerFontCreateFromMemory(const uint8_t* data, size_t size);
ALIMER_API void alimerFontDestroy(Font* font);
ALIMER_API void alimerFontGetMetrics(Font* font, int* ascent, int* descent, int* linegap);
ALIMER_API int alimerFontGetGlyphIndex(Font* font, int codepoint);
ALIMER_API float alimerFontGetScale(Font* font, float size);
ALIMER_API float alimerFontGetScalePixelHeight(Font* font, float height);
ALIMER_API float alimerFontGetKerning(Font* font, int glyph1, int glyph2, float scale);
ALIMER_API void alimerFontGetCharacter(Font* font, int glyph, float scale, int* width, int* height, float* advance, float* offsetX, float* offsetY, int* visible);
ALIMER_API void alimerFontGetPixels(Font* font, uint8_t* dest, int glyph, int width, int height, float scale);

#endif /* ALIMER_FONT_H_ */
