/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tools/library/LoadDynamicLibrary.h"

#if !defined(SK_BUILD_FOR_WIN)

#include <dlfcn.h>

void* SkLoadDynamicLibrary(const char* libraryName, DynamicLibrarySearch search) {
    const int flags =
            search == DynamicLibrarySearch::AlreadyLoaded ? RTLD_NOLOAD | RTLD_NOW : RTLD_LAZY;
    return dlopen(libraryName, flags);
}

void* SkGetProcedureAddress(void* library, const char* functionName) {
    return dlsym(library, functionName);
}

bool SkFreeDynamicLibrary(void* library) {
    return dlclose(library) == 0;
}

#endif//!defined(SK_BUILD_FOR_WIN)
