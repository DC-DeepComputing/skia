/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/SkFontHost_FreeType_bridge.h"

FT_Error FTFunctionProviderNative::FT_New_Library(FT_Memory memory, FTLibrary& library) {
    FT_Library raw_library;
    FT_Error err = ::FT_New_Library(memory, &raw_library);
    if (!err) {
        library.init_native(raw_library);
    }
    return err;
}

void FTFunctionProviderNative::FT_Add_Default_Modules(const FTLibrary& library) {
    ::FT_Add_Default_Modules(library.get_native());
}

void FTFunctionProviderNative::FT_Set_Default_Properties(const FTLibrary& library) {
    ::FT_Set_Default_Properties(library.get_native());
}

FT_Error FTFunctionProviderNative::FT_Library_SetLcdFilter(const FTLibrary& library,
                                                           FT_LcdFilter filter) {
    return ::FT_Library_SetLcdFilter(library.get_native(), filter);
}
