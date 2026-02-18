/*
 * Copyright 2026 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "modules/skottie/include/TextShaper.h"

#import <Foundation/Foundation.h>
#include <limits>

namespace skottie::internal {

// Checks whether the shaper result has any words split across multiple lines.
bool ValidateLinebreaks(const SkString& txt, const skottie::Shaper::Result& res) {
    if (res.fFragments.empty()) {
        return true;
    }

    struct LineSpan { size_t first_index, last_index; };
    std::vector<LineSpan> lines;

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

    SkASSERT(!lines.empty());

    __block auto lines_it  = lines.begin();
    __block auto lines_end = lines.end();
    __block bool is_valid = true;

    NSString *nstxt = @(txt.c_str());
    [nstxt enumerateSubstringsInRange: NSMakeRange(0, [nstxt length])
           options:NSStringEnumerationByWords | NSStringEnumerationLocalized
           usingBlock:^(NSString*, NSRange subrange, NSRange, BOOL* stop) {
        SkASSERT(lines_it != lines_end);

        while (lines_it->last_index < subrange.location) {
            lines_it++;

            if (lines_it == lines_end) {
                is_valid = false;
                *stop = YES;
                return;
            }
        }

        if (subrange.location + subrange.length > lines_it->last_index + 1) {
            is_valid = false;
            *stop = YES;
        }
    }];

    return is_valid;
}

} // namespace skottie::internal
