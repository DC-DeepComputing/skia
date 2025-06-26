/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 #ifndef SkottieAPI_DEFINED
 #define SkottieAPI_DEFINED

 #if !defined(SKOTTIE_IMPLEMENTATION)
    #if defined(SKIA_IMPLEMENTATION)
        #define SKOTTIE_IMPLEMENTATION SKIA_IMPLEMENTATION
    #else
        #define SKOTTIE_IMPLEMENTATION 0
    #endif
#endif

#if !defined(SKOTTIE_DLL) && defined(SKIA_DLL)
    #define SKOTTIE_DLL
#endif

#if !defined(SKOTTIE_API)
    #if defined(SKOTTIE_DLL)
        #if defined(_MSC_VER)
            #if SKOTTIE_IMPLEMENTATION
                #define SKOTTIE_API __declspec(dllexport)
            #else
                #define SKOTTIE_API __declspec(dllimport)
            #endif
        #else
            #define SKOTTIE_API __attribute__((visibility("default")))
        #endif
    #else
        #define SKOTTIE_API
    #endif
#endif

#endif  // SkottieAPI_DEFINED
