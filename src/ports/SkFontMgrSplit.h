/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgrSplit_DEFINED
#define SkFontMgrSplit_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/core/SkTypeface.h"
#include <memory>

// Font match results interface
class FontMatchResults {

};

class FontConfigMatchResults : public FontMatchResults {

};

class Backend {

};

class FreeType : public Backend {

};

template <typename T>
FontMatchResults MatchFamilyStyle(...)

template <typename T>
FontMatchResults MatchFamilyStyleCharacter(...);

template <typename T>
sk_sp<SkTypeface> MakeTypeface(const FontMatchResults& matchResults) {

}

class Example {
  void test1() {

      auto matchResults = MatchFamilyStyle<FontConfigMatchResults>();
      // ...

      auto typeface = MakeTypeface<FreeType>(matchResults);
  }
};

#endif // SkFontMgrSplit_DEFINED