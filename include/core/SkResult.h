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
#include "include/private/base/SkTypeTraits.h"

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

/**
 * SkResultV and SkResultP provide a way to return either a success value or an error similar to
 * C++23's std::expected or Rust's Result. The goal is that if has_value() returns true,
 * a well-formed (and non-null) value can be taken from the result.
 * These are [[nodiscard]] to ensure errors are not accidentally ignored.
 */

/**
 * For standard objects. Ownership is moved into the result and moved out via take().
 * After take() or a move, the result transitions to SkError::kEmptyResult (or the default error).
 */
template <typename T, typename E = SkError>
class [[nodiscard]] SkResultV {
    static_assert(!std::is_pointer_v<T>, "Use SkResultP for pointer types.");
    static_assert(!sk_is_smart_pointer<T>::value, "SkResultV should not hold smart pointers.");
    static_assert(!std::is_same_v<T, E>, "T and E must be different types.");

public:
    // Non-explicit to allow ergonomically returning {val} or {err}.
    constexpr SkResultV(const T& val) : fStorage(val) {}
    constexpr SkResultV(T&& val) : fStorage(std::move(val)) {}
    constexpr SkResultV(E err) : fStorage(err) {}

    // Move-only to ensure clear ownership.
    SkResultV(const SkResultV&) = delete;
    SkResultV& operator=(const SkResultV&) = delete;
    SkResultV(SkResultV&& that) : fStorage(std::move(that.fStorage)) { that.fStorage = E{}; }
    SkResultV& operator=(SkResultV&& that) {
        if (this != &that) {
            fStorage = std::move(that.fStorage);
            that.fStorage = E{};
        }
        return *this;
    }

    bool has_value() const { return std::holds_alternative<T>(fStorage); }
    explicit operator bool() const { return has_value(); }

    /** Moves the value out. The Result transitions to an error state. */
    [[nodiscard]] T take() {
        SkASSERT(has_value());
        auto rv = std::get<0>(std::move(fStorage));
        fStorage = E{};
        return rv;
    }

    const E& error() const {
        SkASSERT(!has_value());
        return std::get<1>(fStorage);
    }

private:
    std::variant<T, E> fStorage;
};

/**
 * If one would want to return a raw, sk_sp, or unique_ptr, use SkResultP instead.
 *
 * If destroyed without being taken, the held value is cleaned up.
 */
template <typename T, typename E = SkError>
class [[nodiscard]] SkResultP {
    static_assert(!std::is_same_v<T, E>, "T and E must be different types.");

public:
    // Non-explicit to allow ergonomically returning {ptr} or {err}.
    SkResultP(T* ptr) : fPtr(ptr), fError() { SkASSERT(fPtr); }
    SkResultP(std::nullptr_t) = delete;
    SkResultP(E err) : fPtr(nullptr), fError(err) {}

    // Move-only to ensure clear ownership.
    SkResultP(const SkResultP&) = delete;
    SkResultP& operator=(const SkResultP&) = delete;
    SkResultP(SkResultP&& that) : fPtr(that.fPtr), fError(std::move(that.fError)) {
        that.fPtr = nullptr;
        that.fError = {};
    }
    SkResultP& operator=(SkResultP&& that) {
        if (this != &that) {
            this->cleanup();
            fPtr = that.fPtr;
            fError = std::move(that.fError);
            that.fPtr = nullptr;
            that.fError = {};
        }
        return *this;
    }

    ~SkResultP() { this->cleanup(); }

    bool has_value() const { return fPtr; }
    explicit operator bool() const { return has_value(); }

    /** Transfers ownership to the caller. */
    [[nodiscard]] sk_sp<T> take() {
        SkASSERT(has_value());
        auto rv = sk_sp<T>(fPtr);
        fPtr = nullptr;
        fError = {};
        return rv;
    }

    /** Transfers ownership to the caller. */
    [[nodiscard]] std::unique_ptr<T> takeUnique() {
        SkASSERT(has_value());
        std::unique_ptr<T> rv(fPtr);
        fPtr = nullptr;
        fError = {};
        return rv;
    }

    const E& error() const {
        SkASSERT(!has_value());
        return fError;
    }

private:
    T* fPtr;
    E fError;

    void cleanup() {
        if (fPtr) {
            if constexpr (std::is_base_of_v<SkRefCntBase, T>) {
                fPtr->unref();
            } else if constexpr (std::is_base_of_v<SkNVRefCnt<T>, T>) {
                fPtr->unref();
            } else {
                delete fPtr;
            }
        }
    }
};

#endif
