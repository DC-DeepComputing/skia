/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKFONTHOST_FREETYPE_BRIDGE_H_
#define SKFONTHOST_FREETYPE_BRIDGE_H_

#include "include/private/base/SkAssert.h"

#include <stdint.h>

#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include <freetype/ftmodapi.h>

template <typename T>
class FTType {
public:
    virtual T get_native() const {
        SkASSERT(false);
        return nullptr;
    }

    virtual uint32_t get_sandboxed() const {
        SkASSERT(false);
        return 0;
    }

    virtual void init_native(T object) { SkASSERT(false); }
    virtual void init_sandboxed(uint32_t offset) { SkASSERT(false); }

    virtual ~FTType() {}
};

using FTLibrary = FTType<FT_Library>;

class FTLibraryNative : public FTLibrary {
public:
    FT_Library get_native() const { return fLibrary; }

    void init_native(FT_Library library) { fLibrary = library; }

    FTLibraryNative() : fLibrary(nullptr) {}

    ~FTLibraryNative() {
        if (fLibrary) {
            FT_Done_Library(fLibrary);
        }
    }

protected:
    FT_Library fLibrary;
};

class FTFunctionProvider {
public:
    virtual FT_Error FT_New_Library(FT_Memory memory, FTLibrary& library) = 0;
    virtual void FT_Add_Default_Modules(const FTLibrary& library) = 0;
    virtual void FT_Set_Default_Properties(const FTLibrary& library) = 0;
    virtual FT_Error FT_Library_SetLcdFilter(const FTLibrary& library, FT_LcdFilter filter) = 0;
    virtual ~FTFunctionProvider() {}
};

class FTFunctionProviderNative : public FTFunctionProvider {
public:
    FT_Error FT_New_Library(FT_Memory memory, FTLibrary& library) override;
    void FT_Add_Default_Modules(const FTLibrary& library) override;
    void FT_Set_Default_Properties(const FTLibrary& library) override;
    FT_Error FT_Library_SetLcdFilter(const FTLibrary& library, FT_LcdFilter filter) override;
    ~FTFunctionProviderNative() {}
};

#endif
