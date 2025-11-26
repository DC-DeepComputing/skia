/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTLazy_DEFINED
#define SkTLazy_DEFINED

#include "include/private/base/SkAssert.h"

#include <optional>
#include <utility>

template <typename T> T* SkOptAddressOrNull(std::optional<T>& object) {
    return object.has_value() ? &object.value() :  nullptr;
}

template <typename T> const T* SkOptAddressOrNull(const std::optional<T>& object) {
    return object.has_value() ? &object.value() :  nullptr;
}

/**
 * A helper built on top of std::optional to do copy-on-first-write. The object is initialized
 * with a const pointer but provides a non-const pointer accessor. The first time the
 * accessor is called (if ever) the object is cloned.
 *
 * In the following example at most one copy of constThing is made:
 *
 * SkTCopyOnFirstWrite<Thing> thing(&constThing);
 * ...
 * function_that_takes_a_const_thing_ptr(thing); // constThing is passed
 * ...
 * if (need_to_modify_thing()) {
 *    thing.writable()->modifyMe(); // makes a copy of constThing
 * }
 * ...
 * x = thing->readSomething();
 * ...
 * if (need_to_modify_thing_now()) {
 *    thing.writable()->changeMe(); // makes a copy of constThing if we didn't call modifyMe()
 * }
 *
 * consume_a_thing(thing); // could be constThing or a modified copy.
 */
template <typename T>
class SkTCopyOnFirstWrite {
public:
    explicit SkTCopyOnFirstWrite(const T& initial) : fObj(&initial) {}

    explicit SkTCopyOnFirstWrite(const T* initial) : fObj(initial) {}

    // Constructor for delayed initialization.
    SkTCopyOnFirstWrite() : fObj(nullptr) {}

    SkTCopyOnFirstWrite(const SkTCopyOnFirstWrite&  that) { *this = that;            }
    SkTCopyOnFirstWrite(      SkTCopyOnFirstWrite&& that) { *this = std::move(that); }

    SkTCopyOnFirstWrite& operator=(const SkTCopyOnFirstWrite& that) {
        fLazy = that.fLazy;
        fObj  = fLazy.has_value() ? &fLazy.value() : that.fObj;
        return *this;
    }

    SkTCopyOnFirstWrite& operator=(SkTCopyOnFirstWrite&& that) {
        fLazy = std::move(that.fLazy);
        fObj  = fLazy.has_value() ? &fLazy.value() : that.fObj;
        return *this;
    }

    // Should only be called once, and only if the default constructor was used.
    void init(const T& initial) {
        SkASSERT(!fObj);
        SkASSERT(!fLazy.has_value());
        fObj = &initial;
    }

    // If not already initialized, in-place instantiates the writable object
    template <typename... Args>
    void initIfNeeded(Args&&... args) {
        if (!fObj) {
            SkASSERT(!fLazy.has_value());
            fObj = &fLazy.emplace(std::forward<Args>(args)...);
        }
    }

    /**
     * Returns a writable T*. The first time this is called the initial object is cloned.
     */
    T* writable() {
        SkASSERT(fObj);
        if (!fLazy.has_value()) {
            fLazy = *fObj;
            fObj = &fLazy.value();
        }
        return &fLazy.value();
    }

    const T* get() const { return fObj; }

    /**
     * Operators for treating this as though it were a const pointer.
     */

    const T *operator->() const { return fObj; }

    operator const T*() const { return fObj; }

    const T& operator *() const { return *fObj; }

private:
    const T*         fObj;
    std::optional<T> fLazy;
};

#endif
