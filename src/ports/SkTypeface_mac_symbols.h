/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_mac_symbols_DEFINED
#define SkTypeface_mac_symbols_DEFINED

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"

// Unicode Plane 16 Supplementary Private Use Area-B base codepoint (U+100000).
// SF Symbols are mapped sequentially into [kSFSymbolsBaseCodepoint, kSFSymbolsBaseCodepoint + count)
// to avoid collisions with standard Unicode and Plane 0/1/15 PUA codepoints.
constexpr SkUnichar kSFSymbolsBaseCodepoint = 0x100000;

// Returns true if the codepoint falls within the valid SF Symbols PUA range on this system.
bool SkIsSFSymbolsCodepoint(SkUnichar codepoint);

// Returns the total number of SF Symbols discovered in CoreGlyphs on this system.
size_t SkGetSFSymbolsCount();

// Returns the Apple system symbol name (e.g. "square.and.arrow.up") for the given PUA codepoint,
// or nullptr if out of range.
const char* SkGetSFSymbolName(SkUnichar codepoint);

// Creates a virtual SkTypeface representing SF Symbols rasterized via NSImage/UIImage system symbols.
sk_sp<SkTypeface> SkMakeTypeface_SFSymbols(const SkFontStyle& style);

#endif // SK_BUILD_FOR_MAC || SK_BUILD_FOR_IOS
#endif // SkTypeface_mac_symbols_DEFINED
