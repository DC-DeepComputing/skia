/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkResult_DEFINED
#define SkResult_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"

#include <memory>
#include <variant>

// Some general purpose errors. Most applications probably don't need
// their own custom errors, so they can use this.
enum class SkError {
    kEmptyResult,
    kInvalidInput,
    kUnsupported,
    kInternalError,
    kOutOfMemory,
};

/** A simplified version of C++23's std::expected. It also draws inspiration from Rust's Result */
template <typename T, typename E = SkError> class SkResultV {
public:
    // We don't want explicit on these, otherwise the syntax to create them
    // requires templating and is way more verbose.
    SkResultV(const T& val) : fStorage(val) {}
    SkResultV(T&& val) : fStorage(std::move(val)) {}
    SkResultV(E err) : fStorage(err) {}

    bool has_value() const { return std::holds_alternative<T>(fStorage); }
    explicit operator bool() const { return has_value(); }

    const T& value() const& {
        SkASSERT(has_value());
        return std::get<0>(fStorage);
    }
    T& value() & {
        SkASSERT(has_value());
        return std::get<0>(fStorage);
    }
    T&& value() && {
        SkASSERT(has_value());
        return std::get<0>(std::move(fStorage));
    }

    const E& error() const& {
        SkASSERT(!has_value());
        return std::get<1>(fStorage);
    }
    E&& error() && {
        SkASSERT(!has_value());
        return std::get<1>(std::move(fStorage));
    }

    const T& operator*() const& { return this->value(); }
    T& operator*() & { return this->value(); }
    T&& operator*() && { return std::move(*this).value(); }

    const T* operator->() const& { return &this->value(); }
    T* operator->() & { return &this->value(); }

private:
    std::variant<T, E> fStorage;
};

template <typename T> static void SkResultDispose(T* ptr) {
    if (ptr) {
        if constexpr (std::is_base_of_v<SkRefCntBase, T>) {
            ptr->unref();
        } else if constexpr (std::is_base_of_v<SkNVRefCnt<T>, T>) {
            ptr->unref();
        } else {
            delete ptr;
        }
    }
}

/** This will either have a non-null pointer or an error. */
template <typename T, typename E = SkError> class SkResultP {
public:
    // We don't want explicit on these, otherwise the syntax to create them
    // requires templating and is way more verbose.
    SkResultP(T* ptr) : fPtr(ptr), fError() { SkASSERT(fPtr); }
    SkResultP(std::nullptr_t) = delete;
    SkResultP(E err) : fPtr(nullptr), fError(err) {}

    // not copyable or assignable
    SkResultP(const SkResultP&) = delete;
    SkResultP& operator=(const SkResultP&) = delete;
    SkResultP& operator=(SkResultP&& that) = delete;

    // movable
    SkResultP(SkResultP&& that) : fPtr(that.fPtr), fError(std::move(that.fError)) {
        that.fPtr = nullptr;
    }

    ~SkResultP() { SkResultDispose(fPtr); }

    bool has_value() const { return fPtr; }
    explicit operator bool() const { return has_value(); }

    sk_sp<T> take() {
        SkASSERT(has_value());
        auto rv = sk_sp<T>(fPtr);
        fPtr = nullptr;
        return std::move(rv);
    }

    std::unique_ptr<T> takeUnique() {
        SkASSERT(has_value());
        auto rv = std::make_unique<T>(fPtr);
        fPtr = nullptr;
        return std::move(rv);
    }

    const E& error() const {
        SkASSERT(!has_value());
        return fError;
    }

    T* operator->() {
        SkASSERT(has_value());
        return fPtr;
    }

private:
    T* fPtr;
    E fError;
};

#endif
