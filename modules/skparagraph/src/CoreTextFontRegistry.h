// Copyright 2026 Google LLC
#ifndef CoreTextFontRegistry_DEFINED
#define CoreTextFontRegistry_DEFINED

#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/SkMutex.h"
#include "src/core/SkTHash.h"
#include "src/utils/mac/SkUniqueCFRef.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <CoreText/CoreText.h>

namespace skia {
namespace textlayout {

class CoreTextFontRegistry {
public:
    static CoreTextFontRegistry& Instance();

    // Returns a retained CTFontDescriptorRef for the typeface
    // Cached by SkTypefaceID; caller takes ownership of the returned reference
    SkUniqueCFRef<CTFontDescriptorRef> getDescriptor(SkTypeface* typeface);

    // Creates a CTFontRef for the given typeface and point size
    // If typeface is null, falls back to Apple system UI font
    // Caller takes ownership of the returned reference
    SkUniqueCFRef<CTFontRef> createFont(SkTypeface* typeface, CGFloat size);

    // Creates a CTFontRef for the given SkFont
    // Caller takes ownership of the returned reference
    SkUniqueCFRef<CTFontRef> createFont(const SkFont& font);

    // Checks if the given typeface is an Apple system font
    static bool isAppleSystemFont(const SkTypeface* typeface);

    // Checks if the given family name is an Apple system font family
    static bool isAppleSystemFont(const char* familyName);

    // Clears the descriptor cache
    void clearCache();

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

#endif  // defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)

#endif  // CoreTextFontRegistry_DEFINED
