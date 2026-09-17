/*
 * Copyright 2024 Google LLC
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

constexpr SkUnichar kSFSymbolsBaseCodepoint = 0x100000;

bool SkIsSFSymbolsCodepoint(SkUnichar codepoint);
size_t SkGetSFSymbolsCount();
const char* SkGetSFSymbolName(SkUnichar codepoint);
sk_sp<SkTypeface> SkMakeTypeface_SFSymbols(const SkFontStyle& style);

#endif // SK_BUILD_FOR_MAC || SK_BUILD_FOR_IOS
#endif // SkTypeface_mac_symbols_DEFINED
