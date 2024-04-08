/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicodeFactoryHelpers_DEFINED
#define SkUnicodeFactoryHelpers_DEFINED

#include "modules/skunicode/include/SkUnicode.h"
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu.h"
#endif

#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_libgrapheme.h"
#endif

#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_icu4x.h"
#endif

#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
#include "modules/skunicode/include/SkUnicode_client.h"
#endif

namespace SkUnicodes {

class Factory : public SkRefCnt {
public:
    virtual void setWordsUtf8(std::vector<SkUnicode::Position> wordsUtf8) { }
    virtual void setGraphemeBreaksUtf8(std::vector<SkUnicode::Position> graphemesUtf8) { }
    virtual void setLineBreaksUtf8(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8) { }
    virtual void clearData() { }
    virtual sk_sp<SkUnicode> MakeUnicode() = 0;
};

#if defined(SK_UNICODE_CLIENT_IMPLEMENTATION)
class ClientFactory final : public Factory {
   public:
      void setWordsUtf8(std::vector<SkUnicode::Position> wordsUtf8) override {
          fWords = std::move(wordsUtf8);
      }
      void setGraphemeBreaksUtf8(std::vector<SkUnicode::Position> graphemesUtf8) override {
          fGraphemeBreaks = std::move(graphemesUtf8);
      }
      void setLineBreaksUtf8(std::vector<SkUnicode::LineBreakBefore> lineBreaksUtf8) override {
          fLineBreaks = std::move(lineBreaksUtf8);
      }
      void clearData() override {
          fWords.clear();
          fGraphemeBreaks.clear();
          fLineBreaks.clear();
      }

      sk_sp<SkUnicode> MakeUnicode() override {
          return SkUnicodes::Client::Make(fText,
                                          std::move(fWords),
                                          std::move(fGraphemeBreaks),
                                          std::move(fLineBreaks));
      }
   private:
      SkSpan<char> fText;
      std::vector<SkUnicode::Position> fWords;
      std::vector<SkUnicode::Position> fGraphemeBreaks;
      std::vector<SkUnicode::LineBreakBefore> fLineBreaks;
};
#else
class UnicodeFactory final : public Factory {
public:
    sk_sp<SkUnicode> MakeUnicode() override {
#if defined(SK_UNICODE_ICU_IMPLEMENTATION)
        return SkUnicodes::ICU::Make();
#endif
#if defined(SK_UNICODE_ICU4X_IMPLEMENTATION)
        return SkUnicodes::ICU4X::Make();
#endif
#if defined(SK_UNICODE_LIBGRAPHEME_IMPLEMENTATION)
        return SkUnicodes::Libgrapheme::Make();
#endif
        return nullptr;
    }
};
#endif

// This convenience function will return a set of callbacks that has the "best" text shaping
// depending on what parts of Skia the client has compiled in. For example, if the clients
// have compiled in SkShaper and a version of SkUnicode, callbacks which produce the
// appropriate types will be returned.
//
// This must be inline (and defined in this header) because the *client* has to compile this code
// with all defines set by *their* dependencies (which may include defines from SkShaper and
// SkUnicode modules).
inline sk_sp<Factory> BestAvailable() {
#ifdef SK_UNICODE_CLIENT_IMPLEMENTATION
    return sk_make_sp<ClientFactory>();
#else
    return sk_make_sp<UnicodeFactory>();
#endif
    return nullptr;
}
};  // namespace SkUnicodes

#endif  // SkUnicodeFactoryHelpers_DEFINED
