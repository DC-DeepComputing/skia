/*
 * Copyright 2026 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/base/SkTo.h"
#include "modules/skottie/include/TextShaper.h"

#import <Foundation/Foundation.h>
#include <limits>

namespace skottie::internal {

bool ValidateLinebreaks(const SkString& txt, const skottie::Shaper::Result& res) {
    struct LineSpan { size_t first_index, last_index; };
    __block std::vector<LineSpan> lines;

    for (const auto& frag : res.fFragments) {
        const uint32_t line_index = frag.fLineIndex;

        if (line_index >= lines.size()) {
            SkASSERT(line_index == lines.size() + 1);
            lines.push_back({std::numeric_limits<size_t>::max(),
                             std::numeric_limits<size_t>::min()});
        }

        LineSpan& line = lines.back();
        for (const auto& cluster : frag.fGlyphs.fClusters) {
            line.first_index = std::min(line.first_index, cluster);
            line.last_index  = std::max(line.last_index, cluster);
        }
    }

    if (lines.empty()) {
        return true;
    }
    __block auto line_it = lines.begin();
    __block bool midword_break = false;

    NSString *nstxt = @(txt.c_str());
    NSRange fullRange = NSMakeRange(0, [nstxt length]);
    [nstxt enumerateSubstringsInRange:fullRange
            options:NSStringEnumerationByWords | NSStringEnumerationLocalized
            usingBlock:^(NSString *substring, NSRange subRange, NSRange enclosingRange, BOOL *stop) {
        if (line_it == lines.end()) {
            return;
        }

        while (line_it->last_index < subRange.location) {
            line_it++;
            if (line_it == lines.end()) {
                midword_break = true;
                return;
            }
        }

        if (subRange.location + subRange.length > line_it->last_index + 1) {
            midword_break = true;
        }
    }];

    return !midword_break;
}

} // namespace skottie::internal
