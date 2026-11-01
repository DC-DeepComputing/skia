/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Dumps SkUnicode_apple code unit flags for a set of tricky strings, so two runs (with and without
// R49_UNI_FAST from ascii_fast_path.patch) can be diffed byte for byte
#include "include/core/SkString.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "modules/skunicode/include/SkUnicode_apple.h"

#include <cstdio>
#include <cstring>
#include <string>

int main() {
    const char* samples[] = {
        "Hello, world! The quick brown fox.",
        "e\u0301 cafe\u0301 a\u0308bc",
        "1\uFE0F\u20E32\uFE0F\u20E3 #\uFE0F\u20E3",
        "\U0001F3F3\uFE0F\u200D\U0001F308 \U0001F469\u200D\U0001F4BB ok",
        "line1\r\nline2\nline3\r\rx\tTab\tEnd",
        "\uD55C\uAE00 \u0939\u093F\u0928\u094D\u0926\u0940 \u0E01\u0E34\u0E19",
        "abc\u0300def\u200Dghi\u00A0jkl\u2028mno",
        "Mixed \u05E9\u05DC\u05D5\u05DD and \u0645\u0631\u062D\u0628\u0627 text.",
        "a\u0301\u0302\u0303b~`!@#$%^&*()_+-=[]{}|;':\",./<>?",
        "",
        "x",
        "\x7F" "\x01" "ABC",
    };
    auto u = SkUnicodes::Apple::Make();
    for (const char* s : samples) {
        std::string copy(s);
        for (int replaceTabs = 0; replaceTabs < 2; ++replaceTabs) {
            std::string t = copy;
            skia_private::TArray<SkUnicode::CodeUnitFlags, true> flags;
            u->computeCodeUnitFlags(t.data(), (int)t.size(), replaceTabs, &flags);
            printf("[%d] ", replaceTabs);
            for (int i = 0; i < flags.size(); ++i) printf("%x ", (unsigned)flags[i]);
            printf("| %s\n", t.c_str());
        }
    }
    return 0;
}
