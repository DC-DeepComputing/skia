/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * A simplified version of C++23's std::expected
 */
#ifndef SkResult_DEFINED
#define SkResult_DEFINED

#include "include/private/base/SkAssert.h"

#include <variant>

template <typename T, typename E>
class SkResult {
public:
    explicit SkResult(const T& val) : storage(val) {}
    explicit SkResult(E err) : storage(err) {}

    bool has_value() const { return std::holds_alternative<T>(storage); }
    explicit operator bool() const { return has_value(); }

    const T& value() const {
        SkASSERT(has_value());
        return std::get<T>(storage);
    }

    const E& error() const {
        SkASSERT(!has_value());
        return std::get<E>(storage);
    }

    const T& operator*() const { return value(); }
    const T* operator->() const { return &value(); }

private:
    std::variant<T, E> storage;
};
#endif