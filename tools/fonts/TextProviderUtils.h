/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef TextProviderUtils_DEFINED
#define TextProviderUtils_DEFINED

#include "modules/skresources/include/SkTextProvider.h"
#include "modules/skshaper/utils/FactoryHelpers.h"
#include "tools/fonts/FontToolUtils.h"

#include <memory>

namespace ToolUtils {

class TextProvider final : public skresources::TextProvider {
public:
    TextProvider(sk_sp<SkFontMgr> mgr): fMgr(std::move(mgr)) {
        SkASSERT(fMgr);
    }

    sk_sp<SkFontMgr> fontMgr() const override {
        return fMgr;
    }

    sk_sp<SkShapers::Factory> textShapingFactory() const override {
        // This call must stay in a header so it is compiled in with whatever defines the user
        // is being built with.
        return SkShapers::BestAvailable();
    }

private:
    sk_sp<SkFontMgr> fMgr;
};

inline sk_sp<skresources::TextProvider> TestTextProvider() {
    return sk_make_sp<TextProvider>(ToolUtils::TestFontMgr());
}

inline sk_sp<skresources::TextProvider> TestTextProvider(sk_sp<SkFontMgr> mgr) {
    return sk_make_sp<TextProvider>(std::move(mgr));
}

}  // namespace ToolUtils

#endif
