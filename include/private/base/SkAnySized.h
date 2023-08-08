/*
 * Copyright 203 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAny_DEFINED
#define SkAny_DEFINED

#include "include/private/base/SkAssert.h"

#include <cstddef>
#include <new>
#include <type_traits>

template <size_t Size>
class SkAnySized {
public:
    SkAnySized() = default;
    ~SkAnySized() {
        this->reset();
    }

    template <typename T, typename... Args>
    void make(Args&&... args) {
        static_assert(sizeof(T) <= Size);
        SkASSERT(!fDestructor && !fValid);
        new (fData) T(std::forward<Args>(args)...);
        if (!std::is_trivially_destructible_v<T>) {
            fDestructor = [](std::byte* data) {
                reinterpret_cast<T*>(data)->~T();
            };
        }
        fValid = true;
    }

    void reset() {
        if (fDestructor) {
            fDestructor(fData);
            fDestructor = nullptr;
        }
        fValid = false;
    }

    template <typename T> const T& get() const {
        SkASSERT(fValid);
        return *std::launder(reinterpret_cast<const T*>(fData));
    }

    template <typename T> T& get() {
        SkASSERT(fValid);
        return *std::launder(reinterpret_cast<T*>(fData));
    }

private:
    using DestructorProc = void(std::byte*);

    alignas(void*) alignas(double) std::byte fData[Size];
    DestructorProc* fDestructor = nullptr;
    bool fValid = false;
};

#endif  // SkAny_DEFINED
