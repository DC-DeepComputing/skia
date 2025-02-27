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

class CodeUnitsPlaceholder : public SkRefCnt {
public:
    static sk_sp<CodeUnitsPlaceholder> Make() {
        return sk_make_sp<CodeUnitsPlaceholder>();
    }
    CodeUnitsPlaceholder() : _unicode(SkUnicodes::Client::Make()) { }

    sk_sp<SkUnicode> getUnicode() { return _unicode; }
private:
    sk_sp<SkUnicode> _unicode;
};

EMSCRIPTEN_BINDINGS(Bidi) {
    class_<SkUnicode>("Bidi")
        .smart_ptr<sk_sp<SkUnicode>>("sk_sp<SkUnicode>")
        .class_function("Make",
                        optional_override([]() -> sk_sp<SkUnicode> {
                            return SkUnicodes::Client::Make();
                        }),
                        allow_raw_pointers())
        .function("_getBidiRegions",
                  optional_override([](SkUnicode& self, JSString jtext, int dir) -> Int32Array {
                      std::string textStorage = jtext.as<std::string>();
                      const char* text = textStorage.data();
                      size_t textCount = textStorage.size();
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
                  }),
                  allow_raw_pointers())

            .function("_reorderVisual",
                optional_override([](SkUnicode& self,
                                   WASMPointerU8 runLevels,
                                   int levelsCount) -> Int32Array {
                    // Convert WASMPointerU8 to std::vector<SkUnicode::BidiLevel>
                    SkUnicode::BidiLevel* data = reinterpret_cast<SkUnicode::BidiLevel*>(runLevels);

                    // The resulting vector
                    std::vector<int32_t> logicalFromVisual;
                    logicalFromVisual.resize(levelsCount);

                    self.reorderVisual(data, levelsCount, logicalFromVisual.data());

                    // Convert std::vector<int32_t> to JSArray
                    JSArray result = emscripten::val::array();
                    for (auto logical : logicalFromVisual) {
                        result.call<void>("push", logical);
                    }
                    return result;
                }),
                allow_raw_pointers());

    class_<CodeUnitsPlaceholder>("CodeUnits")
        .smart_ptr<sk_sp<CodeUnitsPlaceholder>>("sk_sp<CodeUnitsPlaceholder>")
        .class_function("Make",
            optional_override([]() -> sk_sp<CodeUnitsPlaceholder> {
                return CodeUnitsPlaceholder::Make();
            }),
            allow_raw_pointers())

        .function("_compute",
            optional_override([](CodeUnitsPlaceholder& self,
                               JSString jtext) -> Int32Array {
              std::u16string textStorage = jtext.as<std::u16string>();
              char16_t * text = textStorage.data();
              size_t textCount = textStorage.size();
              skia_private::TArray<SkUnicode::CodeUnitFlags, true> flags;
              flags.resize(textCount);
              JSArray result = emscripten::val::array();
              if (!self.getUnicode()->computeCodeUnitFlags(text, textCount, /*replaceTabs=*/false, &flags)) {
                  return result;
              }
              for (auto flag : flags) {
                  result.call<void>("push", (uint16_t)flag);
              }
              return result;
            }),
            allow_raw_pointers());
}