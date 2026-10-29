/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SRC_PARTITION_ALLOC_RAW_PTR_H_
#define SRC_PARTITION_ALLOC_RAW_PTR_H_

#if defined(SK_USE_PARTITION_ALLOC)
#include "partition_alloc/pointers/raw_ptr.h"
#else
#include "src/partition_alloc/noop/raw_ptr.h"
#endif

// C++ template argument deduction does not consider raw_ptr's implicit conversion
// to T*. Forward-declare Skia's ref-counting helpers here (rather than including
// SkRefCnt.h, which would create a core <-> raw_ptr dependency cycle) and provide
// raw_ptr overloads so callers do not have to write .get().
template <typename T> static inline T* SkRef(T* obj);
template <typename T> static inline T* SkSafeRef(T* obj);
template <typename T> class sk_sp;
template <typename T> sk_sp<T> sk_ref_sp(T* obj);
template <typename T> sk_sp<T> sk_ref_sp(const T* obj);

template <typename T, auto Traits> static inline T* SkRef(const raw_ptr<T, Traits>& obj) {
    return SkRef(obj.get());
}

template <typename T, auto Traits> static inline T* SkSafeRef(const raw_ptr<T, Traits>& obj) {
    return SkSafeRef(obj.get());
}

template <typename T, auto Traits> inline sk_sp<T> sk_ref_sp(const raw_ptr<T, Traits>& obj) {
    return sk_ref_sp<T>(obj.get());
}

template <typename T, auto Traits> inline sk_sp<T> sk_ref_sp(const raw_ptr<const T, Traits>& obj) {
    return sk_ref_sp<T>(obj.get());
}

#endif  // SRC_PARTITION_ALLOC_RAW_PTR_H_
