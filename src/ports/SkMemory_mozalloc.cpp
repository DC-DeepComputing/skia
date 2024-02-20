/*
 * Copyright 2011 Google Inc.
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkMalloc.h"

#include "include/core/SkTypes.h"
#include "mozilla/mozalloc.h"
#include "mozilla/mozalloc_abort.h"
#include "mozilla/mozalloc_oom.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
#include <malloc/malloc.h>
#elif defined(SK_BUILD_FOR_ANDROID) || defined(SK_BUILD_FOR_UNIX)
#include <malloc.h>
#elif defined(SK_BUILD_FOR_WIN)
#include <malloc.h>
#endif

void sk_abort_no_print() {
    mozalloc_abort("Abort from sk_abort");
}

void sk_out_of_memory(void) {
    SkDEBUGFAIL("sk_out_of_memory");
    mozalloc_handle_oom(0);
}

void sk_free(void* p) {
    free(p);
}

void* sk_realloc_throw(void* addr, size_t size) {
    return moz_xrealloc(addr, size);
}

void* sk_malloc_flags(size_t size, unsigned flags) {
    if (flags & SK_MALLOC_ZERO_INITIALIZE) {
        return (flags & SK_MALLOC_THROW) ? moz_xcalloc(size, 1) : calloc(size, 1);
    }
    return (flags & SK_MALLOC_THROW) ? moz_xmalloc(size) : malloc(size);
}

size_t sk_malloc_size(void* addr, size_t size) {
    size_t completeSize = size;

    // Use the OS specific calls to find the actual capacity.
    #if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
        // TODO: remove the max, when the chrome implementation of malloc_size doesn't return 0.
        completeSize = std::max(malloc_size(addr), size);
    #elif defined(SK_BUILD_FOR_ANDROID) && __ANDROID_API__ >= 17
        completeSize = malloc_usable_size(addr);
        SkASSERT(completeSize >= size);
    #elif defined(SK_BUILD_FOR_UNIX)
        completeSize = malloc_usable_size(addr);
        SkASSERT(completeSize >= size);
    #elif defined(SK_BUILD_FOR_WIN)
        completeSize = _msize(addr);
        SkASSERT(completeSize >= size);
    #endif

    return completeSize;
}
