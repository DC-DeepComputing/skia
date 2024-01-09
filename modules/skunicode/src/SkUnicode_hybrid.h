/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_hybrid_DEFINED
#define SkUnicode_hybrid_DEFINED

#include <unicode/uscript.h>

std::unique_ptr<SkUnicode> MakeHybridBasedUnicode(std::vector<UScriptCode> forIcu);
std::vector<std::pair<bool, size_t>> breakTextIntoRanges(SkUnicode* hybrid, SkSpan<const char> text);

#endif // SkUnicode_hybrid_DEFINED
