/*
* Copyright 2022 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#include "include/core/SkString.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTHash.h"
#include "include/private/SkTemplates.h"
#include "modules/skunicode/include/SkUnicode.h"
#include "src/utils/SkUTF.h"
#include <vector>
#include <functional>


class SkUnicode_client : public SkUnicode {
public:
    ~SkUnicode_client() override = default;
    void prefill(const SkString& text,
                 std::vector<SkUnicode::BidiRegion> bidiRegions,
                 SkTArray<SkUnicode_client::CodeUnitFlags> codeUnitProperties) {
        fBidiRegions = std::move(bidiRegions);
        fCodeUnitProperties = std::move(codeUnitProperties);
        extractMapping(text,
                       [&](size_t index) {
                            fUTF8IndexForUTF16Index.emplace_back(index);
                       },
                       [&](size_t index) {
                            fUTF16IndexForUTF8Index.emplace_back(index);
                       });
    }

    // For SkShaper
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const uint16_t text[], int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBidiIterator> makeBidiIterator(const char text[],
                                                     int count,
                                                     SkBidiIterator::Direction dir) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(const char locale[],
                                                       BreakType breakType) override;
    std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType breakType) override;
    std::unique_ptr<SkScriptIterator> makeScriptIterator() override;

    // For SkParagraph
    bool getBidiRegions(const char utf8[],
                        int utf8Units,
                        TextDirection dir,
                        std::vector<BidiRegion>* results) override {
        *results = fBidiRegions;
        return true;
    }

    bool getAllCodepoints(const char utf8[],
                          int utf8Units,
                          bool replaceTabs,
                          SkTArray<SkUnicode::CodeUnitFlags>* results) override {
        *results = fCodeUnitProperties;
        return true;
    }

    SkString toUpper(const SkString& str) override {
        SkASSERT(false);
        return fText;
    }

    void reorderVisual(const BidiLevel runLevels[],
                       int levelsCount,
                       int32_t logicalFromVisual[]) override {
        // We have to get this information from the client
        // as a separate step after text layout
    }

private:
    SkString fText;
    std::vector<SkUnicode::BidiRegion> fBidiRegions;
    SkTArray<SkUnicode_client::CodeUnitFlags> fCodeUnitProperties;
    SkTArray<size_t, true> fUTF8IndexForUTF16Index;
    SkTArray<size_t, true> fUTF16IndexForUTF8Index;
};

std::unique_ptr<SkUnicode> SkUnicode::Make() {
    return std::make_unique<SkUnicode_client>();
}
