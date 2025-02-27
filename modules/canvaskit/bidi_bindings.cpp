/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkString.h"

#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skparagraph/src/ParagraphBuilderImpl.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skunicode/include/SkUnicode.h"

#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_client.h"
#endif

#include <string>
#include <vector>

#include <emscripten.h>
#include <emscripten/bind.h>
#include "modules/canvaskit/WasmCommon.h"

using namespace emscripten;
using namespace skia_private;

JSObject JSObjectFromBidiRegion(SkUnicode::BidiRegion & region) {
    JSObject r = emscripten::val::object();
    r.set("start", region.start);
    r.set("end", region.end);
    r.set("level", region.level);
    return r;
}

EMSCRIPTEN_BINDINGS(Bidi) {
    class_<SkUnicode>("Bidi")
            .class_function(
                    "_Make",
                    optional_override([]() -> sk_sp<SkUnicode> {
                        return SkUnicodes::Client::Make(
                                SkString(),
                                std::vector<SkUnicode::Position>(),
                                std::vector<SkUnicode::Position>(),
                                std::vector<SkUnicode::LineBreakBefore>());
                    }),
                    allow_raw_pointers())
            .function("_getBidiRegions", optional_override([](SkUnicode& self,
                                                              std::string text,
                                                              SkUnicode::TextDirection textDirection) -> Int32Array {
                    JSArray result = emscripten::val::array();
                    std::vector<SkUnicode::BidiRegion> regions;
                    if (!self.getBidiRegions(text.data(), text.size(), textDirection, &regions)) {
                        return result;
                    }

                    for (auto region : regions) {
                        result.call<void>("push", JSObjectFromBidiRegion(region));
                    }
                    return result;
                }), allow_raw_pointers())

           .function("_reorderVisual", optional_override([](SkUnicode& self,
                                                            WASMPointerU8 runLevels,
                                                            int levelsCount) -> Int32Array {
                    sk_sp<SkUnicode> clientICU = SkUnicodes::Client::Make();
                    uint8_t* runs = reinterpret_cast<uint8_t*>(runLevels);
                    std::vector<int32_t> logicalFromVisual;
                    self.reorderVisual(runs, levelsCount, logicalFromVisual.data());
                    JSArray result = emscripten::val::array();
                    for (auto logical : logicalFromVisual) {
                        result.call<void>("push", logical);
                    }
                    return result;
                 }), allow_raw_pointers());
}