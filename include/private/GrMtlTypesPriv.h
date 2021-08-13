/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTypesPriv_DEFINED
#define GrMtlTypesPriv_DEFINED

#include "include/gpu/GrTypes.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__

#include <TargetConditionals.h>

#if defined(SK_BUILD_FOR_MAC)
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 110000
#define GR_METAL_SDK_VERSION 230
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 101500
#define GR_METAL_SDK_VERSION 220
#elif __MAC_OS_X_VERSION_MAX_ALLOWED >= 101400
#define GR_METAL_SDK_VERSION 210
#else
#error Must use at least 10.14 SDK to build Metal backend for MacOS
#endif
#else
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 140000 || __TV_OS_VERSION_MAX_ALLOWED >= 140000
#define GR_METAL_SDK_VERSION 230
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 130000 || __TV_OS_VERSION_MAX_ALLOWED >= 130000
#define GR_METAL_SDK_VERSION 220
#elif __IPHONE_OS_VERSION_MAX_ALLOWED >= 120000 || __TV_OS_VERSION_MAX_ALLOWED >= 120000
#define GR_METAL_SDK_VERSION 210
#else
#error Must use at least 12.00 SDK to build Metal backend for iOS
#endif
#endif

#endif  // __APPLE__

#endif  // GrMtlTypesPriv_DEFINED
