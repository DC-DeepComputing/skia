#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkTypeface.h"
#include "tools/Resources.h"

const char kAhemName[] = "fonts/Ahem.ttf";
const char kAhemText[] = "X";
const char kZeroText[] = "0";

int main(int argc, char** argv) {
    sk_sp<SkTypeface> ahemTypeface(MakeResourceAsTypeface(kAhemName));
    SkASSERT(ahemTypeface);

    SkFont ahemFont(ahemTypeface, 25.0f);

    SkFontHinting modes[] = {
            SkFontHinting::kNone,
            SkFontHinting::kSlight,
            SkFontHinting::kNormal,
            SkFontHinting::kFull,
    };

    for (auto hintMode : modes) {
        ahemFont.setHinting(hintMode);
        SkScalar xAdvance = ahemFont.measureText(
                kAhemText, strlen(kAhemText), SkTextEncoding::kUTF8, nullptr, nullptr);
        SkScalar zeroAdvance = ahemFont.measureText(
                kZeroText, strlen(kZeroText), SkTextEncoding::kUTF8, nullptr, nullptr);
        printf("Hint mode: %d - 0 width: %f - Advance width: %f\n",
               hintMode,
               zeroAdvance,
               xAdvance);
    }
}
