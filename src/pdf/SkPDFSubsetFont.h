// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFSubsetFont_DEFINED
#define SkPDFSubsetFont_DEFINED

#include "include/docs/SkPDFDocument.h"
#include "include/core/SkTypeface.h"
#include "src/pdf/SkPDFGlyphUse.h"

sk_sp<SkData> SkPDFSubsetFont(const SkTypeface& typeface,
                              const SkPDFGlyphUse& glyphUsage,
                              SkPDF::Metadata::Subsetter subsetter,
                              const char* fontName);

#endif  // SkPDFSubsetFont_DEFINED
