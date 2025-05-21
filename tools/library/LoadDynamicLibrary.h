/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSLibrary_DEFINED
#define SkOSLibrary_DEFINED

// Where to find the library.
enum class DynamicLibrarySearch {
    // The library should be searched for in the default paths
    Default,
    // The library is expected to already be pre-loaded
    AlreadyLoaded,
};

void* SkLoadDynamicLibrary(const char* libraryName,
                           DynamicLibrarySearch search = DynamicLibrarySearch::Default);
void* SkGetProcedureAddress(void* library, const char* functionName);
bool  SkFreeDynamicLibrary(void* library);

#endif
