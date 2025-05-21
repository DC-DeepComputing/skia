/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/library/LoadDynamicLibrary.h"

#if defined(SK_BUILD_FOR_WIN)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

void* SkLoadDynamicLibrary(const char* libraryName, DynamicLibrarySearch search) {
    switch (search) {
        case DynamicLibrarySearch::AlreadyLoaded:
            return GetModuleHandleA(libraryName);
        // Note: there is no other option than listed, but `default` is added because msvc is too
        // basic.
        default:
        case DynamicLibrarySearch::Default:
            return LoadLibraryA(libraryName);
    }
}

void* SkGetProcedureAddress(void* library, const char* functionName) {
    return reinterpret_cast<void*>(::GetProcAddress((HMODULE)library, functionName));
}

bool SkFreeDynamicLibrary(void* library) {
    return FreeLibrary((HMODULE)library);
}

#endif//defined(SK_BUILD_FOR_WIN)
