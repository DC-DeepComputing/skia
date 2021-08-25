// Copyright 2021 Google LLC.
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
#include "tests/Test.h"
#include "tools/Resources.h"

#include "experimental/sktext/include/Text.h"

#include <string.h>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct GrContextOptions;

#define VeryLongCanvasWidth 1000000
#define TestCanvasWidth 1000
#define TestCanvasHeight 600

#define DEF_TEST_DISABLED(name, reporter) \
static void test_##name(skiatest::Reporter* reporter, const GrContextOptions&); \
skiatest::TestRegistry name##TestRegistry(skiatest::Test(#name, false, test_##name)); \
void test_##name(skiatest::Reporter* reporter, const GrContextOptions&) { /* SkDebugf("Disabled:"#name "\n"); */ } \
void disabled_##name(skiatest::Reporter* reporter, const GrContextOptions&)

using namespace skia::text;

namespace {
    bool operator==(SkSpan<const char16_t> a, SkSpan<const char16_t> b) {
        if (a.size() != b.size()) {
            return false;
        }
        for (size_t i = 0; i < a.size(); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }
}

DEF_TEST(SkText_UnicodeText_Flags, reporter) {
    REPORTER_ASSERT(reporter, true);
    //                       01234567890  1234567890
    std::u16string utf16(u"Hello word\nHello world");
    UnicodeText unicodeText16(SkUnicode::Make(), SkSpan<uint16_t>((uint16_t*)utf16.data(), utf16.size()));
    UnicodeText unicodeText8(SkUnicode::Make(), SkString("Hello word\nHello world"));

    REPORTER_ASSERT(reporter, unicodeText16.getText16() == unicodeText8.getText16());
    for (size_t i = 0; i < unicodeText16.getText16().size(); ++i) {
        if (i == 10) {
            REPORTER_ASSERT(reporter, unicodeText16.hasProperty(i, CodeUnitFlags::kHardLineBreakBefore));
            REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(i, CodeUnitFlags::kHardLineBreakBefore));
        } else {
            REPORTER_ASSERT(reporter, unicodeText16.hasProperty(i, CodeUnitFlags::kGraphemeStart));
            REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(i, CodeUnitFlags::kGraphemeStart));
        }
    }

    auto space1 = utf16.find_first_of(u" ");
    auto space2 = utf16.find_last_of(u" ");

    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space1, CodeUnitFlags::kPartOfWhiteSpace));
    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space1 + 1, CodeUnitFlags::kSoftLineBreakBefore));
    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space2, CodeUnitFlags::kPartOfWhiteSpace));
    REPORTER_ASSERT(reporter, unicodeText16.hasProperty(space2 + 1, CodeUnitFlags::kSoftLineBreakBefore));

    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space1, CodeUnitFlags::kPartOfWhiteSpace));
    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space1 + 1, CodeUnitFlags::kSoftLineBreakBefore));
    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space2, CodeUnitFlags::kPartOfWhiteSpace));
    REPORTER_ASSERT(reporter, unicodeText8 .hasProperty(space2 + 1, CodeUnitFlags::kSoftLineBreakBefore));
}

// TODO: Test RTL text