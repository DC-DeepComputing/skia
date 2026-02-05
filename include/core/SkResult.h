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
#include <type_traits>
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

template <typename T, typename = void>
struct is_smart_pointer : std::false_type {};

template <typename T>
struct is_smart_pointer<T, std::void_t<typename T::element_type>> : std::true_type {};

/** A simplified version of C++23's std::expected. It also draws inspiration from Rust's Result */
template <typename T, typename E = SkError>
class SkResultV {
    static_assert(!std::is_pointer_v<T>, "Use SkResultP for pointer types.");
    static_assert(!is_smart_pointer<T>::value, "SkResultV should not hold smart pointers.");
public:
    // We don't want explicit on these, otherwise the syntax to create them
    // requires templating and is way more verbose.
    SkResultV(const T& val) : fStorage(val) {}
    SkResultV(T&& val) : fStorage(std::move(val)) {}
    SkResultV(E err) : fStorage(err) {}

    // Move-only
    SkResultV(const SkResultV&) = delete;
    SkResultV& operator=(const SkResultV&) = delete;
    SkResultV(SkResultV&&) = default;
    SkResultV& operator=(SkResultV&&) = default;

    bool has_value() const { return std::holds_alternative<T>(fStorage); }
    explicit operator bool() const { return has_value(); }

    T take() {
        SkASSERT(has_value());
        return std::get<0>(std::move(fStorage));
    }

    const E& error() const {
        SkASSERT(!has_value());
        return std::get<1>(fStorage);
    }

    const T& operator*() const {
        SkASSERT(has_value());
        return std::get<0>(fStorage);
    }
    T& operator*() {
        SkASSERT(has_value());
        return std::get<0>(fStorage);
    }

    const T* operator->() const { return &**this; }
    T* operator->() { return &**this; }

private:
    std::variant<T, E> fStorage;
};

template <typename T>
static void SkResultDispose(T* ptr) {
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

template <typename T, typename E = SkError>
class SkResultP {
public:
    // We don't want explicit on these, otherwise the syntax to create them
    // requires templating and is way more verbose.
    SkResultP(T* ptr) : fPtr(ptr), fError() {
        SkASSERT(fPtr);
    }
    SkResultP(std::nullptr_t) = delete;
    SkResultP(E err) : fPtr(nullptr), fError(err) {}

    // not copyable
    SkResultP(const SkResultP&) = delete;
    SkResultP& operator=(const SkResultP&) = delete;

    // movable
    SkResultP(SkResultP&& that) : fPtr(that.fPtr), fError(std::move(that.fError)) {
        that.fPtr = nullptr;
    }
    SkResultP& operator=(SkResultP&& that) {
        if (this != &that) {
            SkResultDispose(fPtr);
            fPtr = that.fPtr;
            fError = std::move(that.fError);
            that.fPtr = nullptr;
        }
        return *this;
    }

    ~SkResultP() {
        SkResultDispose(fPtr);
    }

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
        std::unique_ptr<T> rv(fPtr);
        fPtr = nullptr;
        return rv;
    }

    const E& error() const {
        SkASSERT(!has_value());
        return fError;
    }

    const T& operator*() const {
        SkASSERT(has_value());
        return *fPtr;
    }
    T& operator*() {
        SkASSERT(has_value());
        return *fPtr;
    }

    const T* operator->() const {
        SkASSERT(has_value());
        return fPtr;
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
