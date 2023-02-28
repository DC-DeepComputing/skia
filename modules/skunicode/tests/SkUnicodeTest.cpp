/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageEncoder.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkOSFile.h"
#include "src/utils/SkOSPath.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <thread>

#include "modules/skunicode/include/SkUnicode.h"

UNIX_ONLY_TEST(SkUnicode_GetWords, reporter) {
    SkString text("1 22 333 4444 55555 666666 7777777");

    auto icu4x = SkUnicode::MakeIcu4xBasedUnicode();
    auto icu = SkUnicode::MakeIcuBasedUnicode();
    auto client = SkUnicode::MakeTestClientUnicode(SkSpan<char>(text.data(), text.size()));

    std::vector<SkUnicode::Position> icu4x_results;
    std::vector<SkUnicode::Position> icu_results;
    std::vector<SkUnicode::Position> client_results;
    auto icu4x_result = icu4x->getWords(text.data(), text.size(), "en", &icu4x_results);
    auto icu_result = icu->getWords(text.data(), text.size(), "en", &icu_results);
    auto client_result = icu->getWords(text.data(), text.size(), "en", &client_results);

    REPORTER_ASSERT(reporter, icu4x_result && icu_result && client_result);
    REPORTER_ASSERT(reporter, icu4x_results.size() == 13 &&
                              icu_results.size() == 13 &&
                              client_results.size() == 13);
    for (auto i = 0; i < 13; ++i) {
        REPORTER_ASSERT(reporter, icu4x_results[i] ==icu_results[i] &&
                                  client_results[i] ==icu_results[i]);
    }
};
