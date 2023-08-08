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

template <typename Base, size_t Size>
class SkAnySized {
public:
    SkAnySized() = default;
    ~SkAnySized() {
        this->reset();
    }

    SkAnySized(const SkAnySized&) = delete;
    SkAnySized& operator=(const SkAnySized&) = delete;
    SkAnySized(SkAnySized&&) = delete;
    SkAnySized& operator=(SkAnySized&&) = delete;

    template <typename T, typename... Args>
    void make(Args&&... args) {
        static_assert(std::is_base_of_v<Base, T>);
        static_assert(sizeof(T) <= Size);
        SkASSERT(!fValid);
        new (fData) T(std::forward<Args>(args)...);
        fValid = true;
    }

    void reset() {
        if (fValid) {
            this->get()->~Base();
        }
        fValid = false;
    }

    bool valid() const { return fValid; }

    const Base* get() const {
        SkASSERT(fValid);
        return std::launder(reinterpret_cast<const Base*>(fData));
    }

    Base* get() {
        SkASSERT(fValid);
        return std::launder(reinterpret_cast<Base*>(fData));
    }

    Base* operator->() { return this->get(); }
    const Base* operator->() const { return this->get(); }

private:
    alignas(void*) alignas(double) std::byte fData[Size];
    bool fValid = false;
};

#endif  // SkAny_DEFINED
