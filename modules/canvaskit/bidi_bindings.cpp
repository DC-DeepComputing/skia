/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkString.h"
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

void JSArrayFromBidiRegions(JSArray& array, std::vector<SkUnicode::BidiRegion>& regions) {
    for (auto region : regions) {
        array.call<void>("push", region.start);
        array.call<void>("push", region.end);
        array.call<void>("push", (int32_t)region.level);
    }
}

EMSCRIPTEN_BINDINGS(Bidi) {
    class_<SkUnicode>("Bidi")
            .smart_ptr<sk_sp<SkUnicode>>("sk_sp<SkUnicode>")
            .class_function(
                    "Make",
                    optional_override([]() -> sk_sp<SkUnicode> {
                        return SkUnicodes::Client::Make(
                                SkString(),
                                std::vector<SkUnicode::Position>(),
                                std::vector<SkUnicode::Position>(),
                                std::vector<SkUnicode::LineBreakBefore>());
                    }),
                    allow_raw_pointers())
            .function("_getBidiRegions", optional_override([](SkUnicode& self,
                                                              JSString jtext,
                                                              int dir) -> Int32Array {
                    std::string textStorage = jtext.as<std::string>();
                    const char* text = textStorage.data();
                    size_t      textCount = textStorage.size();
                    JSArray result = emscripten::val::array();
                    std::vector<SkUnicode::BidiRegion> regions;
                    SkUnicode::TextDirection textDirection =
                            dir == 0 ? SkUnicode::TextDirection::kLTR
                                     : SkUnicode::TextDirection::kRTL;
                    if (!self.getBidiRegions(text, textCount, textDirection, &regions)) {
                        return result;
                    }
                    JSArrayFromBidiRegions(result, regions);
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