/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "src/ports/SkFontMgr_custom.h"

class EmptyFontLoader : public SkFontMgr_Custom_FreeType::SystemFontLoader {
public:
    EmptyFontLoader() { }

    void loadSystemFonts(const SkFontScanner* scanner,
                         SkFontMgr_Custom_FreeType::Families* families) const override
    {
        SkFontStyleSet_Custom* family = new SkFontStyleSet_Custom(SkString());
        families->push_back().reset(family);
        family->appendTypeface(sk_make_sp<SkTypeface_Empty>());
    }

};
