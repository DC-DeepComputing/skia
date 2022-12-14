/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAPI_DEFINED
#define SkAPI_DEFINED

// If we are compiling Skia is being as a DLL, we need to be sure to export all of our public
// APIs to that DLL. If a client is using Skia which was compiled as a DLL, we need to instruct
// the linker to use the symbols from that DLL. This is the goal of the SK_API define.
#if !defined(SK_API)
    #if defined(SKIA_DLL)
        #if defined(_MSC_VER)
            #if SKIA_IMPLEMENTATION
                #define SK_API __declspec(dllexport)
            #else
                #define SK_API __declspec(dllimport)
            #endif
        #else
            #define SK_API __attribute__((visibility("default")))
        #endif
    #else
        #define SK_API
    #endif
#endif

// SK_SPI is functionally identical to SK_API, but used within src to clarify that it's less stable
#if !defined(SK_SPI)
    #define SK_SPI SK_API
#endif

#endif
