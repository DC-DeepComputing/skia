/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTypeface_win_DEFINED
#define SkTypeface_win_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#ifdef UNICODE
typedef struct tagLOGFONTW LOGFONTW;
typedef LOGFONTW LOGFONT;
#else
typedef struct tagLOGFONTA LOGFONTA;
typedef LOGFONTA LOGFONT;
#endif  // UNICODE

/**
 *  Like the other Typeface create methods, this returns a new reference to the
 *  corresponding typeface for the specified logfont. The caller is responsible
 *  for calling unref() when it is finished.
 */
SK_API sk_sp<SkTypeface> SkCreateTypefaceFromLOGFONT(const LOGFONT&);

/**
 *  Copy the LOGFONT associated with this typeface into the lf parameter. Note
 *  that the lfHeight will need to be set afterwards, since the typeface does
 *  not track this (the paint does).
 *  typeface may be NULL, in which case we return the logfont for the default font.
 */
SK_API void SkLOGFONTFromTypeface(const SkTypeface* typeface, LOGFONT* lf);

/**
  *  Set an optional callback to ensure that the data behind a LOGFONT is loaded.
  *  This will get called if Skia tries to access the data but hits a failure.
  *  Normally this is null, and is only required if the font data needs to be
  *  remotely (re)loaded.
  */
SK_API void SkTypeface_SetEnsureLOGFONTAccessibleProc(void (*)(const LOGFONT&));

// Experimental!
//
class SkFontMgr;
struct IDWriteFactory;
struct IDWriteFontCollection;
struct IDWriteFont;
struct IDWriteFontFace;
struct IDWriteFontFallback;
struct IDWriteFontFamily;
using WCHAR = wchar_t;
using UINT32 = unsigned int;

SK_API sk_sp<SkFontMgr> SkFontMgr_New_GDI();
SK_API sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory = nullptr,
                                                  IDWriteFontCollection* collection = nullptr);
SK_API sk_sp<SkFontMgr> SkFontMgr_New_DirectWrite(IDWriteFactory* factory,
                                                  IDWriteFontCollection* collection,
                                                  IDWriteFontFallback* fallback);

class SkNativeFontCollection_DirectWrite : public SkRefCnt {
public:
    SK_API static sk_sp<SkNativeFontCollection_DirectWrite> Make(IDWriteFactory* factory,
                                                                 IDWriteFontCollection* collection,
                                                                 IDWriteFontFallback* fallback);

    virtual int countFamilies() const = 0;
    virtual void getFamilyName(int index, SkString* familyName) const = 0;
    virtual IDWriteFontFamily* createStyleSet(int index) const = 0;
    virtual IDWriteFontFamily* matchFamily(const char familyName[]) const = 0;

    class Match {
    public:
        Match(std::nullptr_t) {}
        Match(IDWriteFontFamily* family, IDWriteFont* font, IDWriteFontFace* face)
                : fFamily(family), fFont(font), fFace(face) {}
        Match(const Match&) = delete;
        Match& operator=(const Match&) = delete;
        Match(Match&& that);
        Match& operator=(Match&& that);
        ~Match(); // Will Release pointers if non-nullptr

        explicit operator bool() { return fFamily && fFont && fFace; }

        IDWriteFontFamily* getFamily() const { return fFamily; }
        IDWriteFont* getFont() const { return fFont; }
        IDWriteFontFace* getFace() const { return fFace; }

        IDWriteFontFamily* releaseFamily() {
            IDWriteFontFamily* family = fFamily;
            fFamily = nullptr;
            return family;
        }
        IDWriteFont* releaseFont() {
            IDWriteFont* font = fFont;
            fFont = nullptr;
            return font;
        }
        IDWriteFontFace* releaseFace() {
            IDWriteFontFace* face = fFace;
            fFace = nullptr;
            return face;
        }

    private:
        // These are classic COM owning bare pointers.
        IDWriteFontFamily* fFamily = nullptr;
        IDWriteFont* fFont = nullptr;
        IDWriteFontFace* fFace = nullptr;
    };
    virtual Match matchFamilyStyle(IDWriteFontFamily*,
                                   const SkFontStyle& fontstyle) const = 0;
    virtual Match matchFamilyStyleString(const char familyName[], const SkFontStyle&,
                                         const char* bcp47[], int bcp47Count,
                                         const WCHAR* str, UINT32 strLen) const = 0;
    virtual Match legacyMakeTypeface(const char familyName[], SkFontStyle) const = 0;

    virtual IDWriteFactory* factory() const = 0;
    virtual WCHAR* locale() const = 0;
};

#endif  // SK_BUILD_FOR_WIN
#endif  // SkTypeface_win_DEFINED
