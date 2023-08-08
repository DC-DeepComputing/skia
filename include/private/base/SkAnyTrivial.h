/*
 * Copyright 203 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAny_DEFINED
#define SkAny_DEFINED

#include <cstring>
#include <new>
#include <type_traits>

template <size_t Size>
struct SkAnyTrivial {
    SkAnyTrivial() = default;
    ~SkAnyTrivial() = default;

    SkAnyTrivial(const SkAnyTrivial& that) {
        std::memcpy(fData, that.fData, Size);
    }

    SkAnyTrivial& operator=(const SkAnyTrivial& rhs) {
        std::memcpy(fData, rhs.fData, Size);
        return *this;
    }

    template <typename T, typename... Args>
    void init(Args&&... args) {
        static_assert(sizeof(T) <= Size);
        static_assert(std::is_trivial_v<T>);
        new (fData) T(std::forward<Args>(args)...);
    }

    template <typename T> const T& get() const {
        return *std::launder(reinterpret_cast<const T*>(fData));
    }

    template <typename T> T& get() {
        return *std::launder(reinterpret_cast<T*>(fData));
    }

    alignas(void*) alignas(double) std::byte fData[Size];
};

#endif  // SkAny_DEFINED
