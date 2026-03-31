/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/jsonreader/SkJSONReader.h"
#include "tests/Test.h"

#include <clocale>
#include <cmath>
#include <cstring>
#include <string>

namespace {

class ScopedLocale {
public:
    explicit ScopedLocale(const char* locale) {
        const char* prev = std::setlocale(LC_NUMERIC, locale);
        if (prev) {
            fSavedLocale = prev;
        }
    }
    ~ScopedLocale() { std::setlocale(LC_NUMERIC, fSavedLocale.c_str()); }
    ScopedLocale(const ScopedLocale&) = delete;
    ScopedLocale& operator=(const ScopedLocale&) = delete;
private:
    std::string fSavedLocale = "C";
};

struct NumberTestCase {
    const char* json;
    const char* key;
    float expected;
};

void VerifyFloatParsing(skiatest::Reporter* reporter, const NumberTestCase& tc) {
    skjson::DOM dom(tc.json, strlen(tc.json));
    const auto& root = dom.root();
    if (!root.is<skjson::ObjectValue>()) {
        ERRORF(reporter, "Failed to parse: %s", tc.json);
        return;
    }

    const skjson::NumberValue* val = root[tc.key];
    if (!val) {
        ERRORF(reporter, "Value not a number for: %s", tc.json);
        return;
    }

    const float parsed = static_cast<float>(**val);
    const float tolerance = std::abs(tc.expected) * 1e-6f + 1e-10f;
    REPORTER_ASSERT(reporter,
                    std::abs(parsed - tc.expected) <= tolerance,
                    "Mismatch for %s: got %g, expected %g",
                    tc.json,
                    parsed,
                    tc.expected);
}

}  // namespace

// Validation that skjson::DOM correctly parses floating-point numbers
// in scientific notation (e.g. "1.5e-3"). It shouldn't fail when system locale
// is using a comma as the decimal separator.
DEF_TEST(SkjsonFloatParsing_ScientificNotation, reporter) {
    ScopedLocale commaLocale("de_DE.UTF-8");

    static const NumberTestCase cases[] = {
            {"{\"v\": 1e0}", "v", 1.0f},
            {"{\"v\": 1e1}", "v", 10.0f},
            {"{\"v\": 1e+1}", "v", 10.0f},
            {"{\"v\": 1e-1}", "v", 0.1f},
            {"{\"v\": -1e1}", "v", -10.0f},
            {"{\"v\": -1e-1}", "v", -0.1f},
            {"{\"v\": 3.14e2}", "v", 314.0f},
            {"{\"v\": 3.14e-2}", "v", 0.0314f},
            {"{\"v\": 1.23E4}", "v", 12300.0f},
            {"{\"v\": 1.23E-4}", "v", 0.000123f},
            {"{\"v\": 0e0}", "v", 0.0f},
            {"{\"v\": 5E+0}", "v", 5.0f},
            {"{\"v\": 1.0e+2}", "v", 100.0f},
            {"{\"v\": 2.5e-3}", "v", 0.0025f},
            {"{\"v\": 9.99e10}", "v", 9.99e10f},
            {"{\"v\": -7.5E-5}", "v", -7.5e-5f},
    };

    for (const auto& tc : cases) {
        VerifyFloatParsing(reporter, tc);
    }
}
