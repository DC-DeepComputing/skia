/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/oss_fuzz/FuzzSkSLShared.h"
#include "include/core/SkString.h"

#include <string_view>

static bool is_control_character(uint8_t c) {
    return !(c >= 32 || c == '\r' || c == '\n' || c == '\t');
}

static constexpr std::string_view kSkSLTokens[] = {
        " true ",
        " false ",
        " if ",
        " @if ",
        " else ",
        " for ",
        " while ",
        " do ",
        " switch ",
        " @switch ",
        " case ",
        " default ",
        " break ",
        " continue ",
        " discard ",
        " return ",
        " in ",
        " out ",
        " inout ",
        " uniform ",
        " const ",
        " flat ",
        " noperspective ",
        " inline ",
        " noinline ",
        " $pure ",
        " readonly ",
        " writeonly ",
        " buffer ",
        " struct ",
        " layout ",
        " highp ",
        " mediump ",
        " lowp ",
        " $es3 ",
        " $export ",
        " threadgroup ",
        " << ",
        " >> ",
        " && ",
        " || ",
        " ^^ ",
        " == ",
        " != ",
        " <= ",
        " >= ",
        " += ",
        " -= ",
        " *= ",
        " /= ",
        " %= ",
        " <<= ",
        " >>= ",
        " &= ",
        " |= ",
        " ^= ",
        " ++ ",
        " -- ",
        " //",
        " /*",
        "*/ ",
        " float",
        " half",
        " int",
        " uint",
        " short",
        " ushort",
        " bool",
        " void ",
        " vec",
        " ivec",
        " bvec",
        " mat",
        " Attributes ",
        " Varyings ",
};

void TransformInputToSkSL(SkSpan<const uint8_t>* data,
                          SkString* sksl,
                          ControlCharacterBehavior controlCharacters,
                          HighBitBehavior highBits) {
    size_t index = 0;
    while (index < data->size()) {
        uint8_t c = (*data)[index++];
        if (c < 128) {
            // We are in the ASCII range.
            if (controlCharacters == ControlCharacterBehavior::kStop && is_control_character(c)) {
                break;
            } else {
                char ascii = c;
                sksl->append(&ascii, /*size=*/1);
            }
        } else {
            // We are outside of the ASCII range.
            if (highBits == HighBitBehavior::kStop) {
                break;
            } else {
                // Replace this byte with an arbitrary SkSL token.
                c %= std::size(kSkSLTokens);
                sksl->append(kSkSLTokens[c]);
            }
        }
    }

    // Trim the consumed data off of the span.
    *data = data->subspan(index);
}
