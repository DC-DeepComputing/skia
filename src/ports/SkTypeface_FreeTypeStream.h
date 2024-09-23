/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_FreetypeStream_DEFINED
#define SkTypeface_FreetypeStream_DEFINED

#include "src/ports/SkTypeface_FreeType_priv.h"

class SkTypeface_FreeTypeStream : public SkTypeface_FreeType {
public:
    SkTypeface_FreeTypeStream(std::unique_ptr<SkFontData> fontData, const SkString familyName,
                              const SkFontStyle& style, bool isFixedPitch);
    ~SkTypeface_FreeTypeStream() override;

protected:
    void onGetFamilyName(SkString* familyName) const override;
    void onGetFontDescriptor(SkFontDescriptor*, bool* serialize) const override;
    std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override;
    std::unique_ptr<SkFontData> onMakeFontData() const override;
    sk_sp<SkTypeface> onMakeClone(const SkFontArguments&) const override;

private:
    const SkString fFamilyName;
    const std::unique_ptr<const SkFontData> fData;
};

#endif // SkTypeface_FreetypeStream_DEFINED
