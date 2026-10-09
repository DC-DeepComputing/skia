// Copyright 2026 Google LLC
#ifndef CoreTextFontRegistry_DEFINED
#define CoreTextFontRegistry_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMutex.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "src/core/SkTHash.h"

#if defined(SK_PARAGRAPH_NATIVE_CORETEXT)
#include "src/utils/mac/SkUniqueCFRef.h"
#include <CoreText/CoreText.h>

namespace skia {
namespace textlayout {

class CoreTextFontRegistry {
public:
    static CoreTextFontRegistry& Instance();

    // In-process descriptor cache indexed by SkTypefaceID to eliminate fontd IPC during run attribution
    SkUniqueCFRef<CTFontDescriptorRef> getDescriptor(SkTypeface* typeface);

    // Resolves optical sizing and symbolic traits via .AppleSystemUIFont when typeface is a system UI font
    SkUniqueCFRef<CTFontRef> createFont(SkTypeface* typeface, CGFloat size);

    // System UI fonts require dynamic descriptor synthesis to preserve cascade lists across scripts
    static bool isAppleSystemFont(const SkTypeface* typeface);

    static bool isAppleSystemFont(const char* familyName);

private:
    CoreTextFontRegistry() = default;
    ~CoreTextFontRegistry() = default;
    CoreTextFontRegistry(const CoreTextFontRegistry&) = delete;
    CoreTextFontRegistry& operator=(const CoreTextFontRegistry&) = delete;

    SkUniqueCFRef<CTFontDescriptorRef> createDescriptorInternal(SkTypeface* typeface);

    SkMutex fMutex;
    skia_private::THashMap<SkTypefaceID, SkUniqueCFRef<CTFontDescriptorRef>>
            fDescriptorCache SK_GUARDED_BY(fMutex);
};

}  // namespace textlayout
}  // namespace skia

#endif  // defined(SK_PARAGRAPH_NATIVE_CORETEXT)

#endif  // CoreTextFontRegistry_DEFINED
