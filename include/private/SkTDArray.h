/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTDArray_DEFINED
#define SkTDArray_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"

#include <algorithm>
#include <cstddef>
#include <climits>
#include <initializer_list>
#include <tuple>
#include <utility>

class SK_SPI SkTDStorage {
public:
    SkTDStorage() = default;
    SkTDStorage(const SkTDStorage& that) = delete;
    SkTDStorage& operator= (const SkTDStorage& that) = delete;
    SkTDStorage(SkTDStorage&& that);
    SkTDStorage& operator= (SkTDStorage&& that);
    ~SkTDStorage();

    void reset();

    void assign(const void* src, int count, size_t sizeOfT);

    bool empty() const { return fCount == 0; }
    void clear() { fCount = 0; }
    int count() const { return fCount; }

    // If newCount is > reserved then increase the size with added space using a heuristic.
    // This never reduces used space.
    void resize(int newCount, size_t sizeOfT);
    int decreaseCount() {
        SkASSERT(0 < fCount);
        fCount -= 1;
        return fCount;
    }

    size_t size_bytes(size_t sizeOfT) const;

    int capacity() const { return fReserve; }
    // Note: this takes a size_t to centralize size checking.
    void reserve(size_t newReserve, size_t sizeOfT);

    void shrinkToFit(size_t sizeOfT);
    void swap(SkTDStorage& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
    }
    template <typename T> T* data() const { return reinterpret_cast<T*>(fStorage); }

    void* erase(int index, int count, size_t sizeOfT);
    void* replaceWithEndErase(int index, size_t sizeOfT);

    void* prepend(size_t sizeOfT);
    void* append(size_t sizeOfT);
    void* append(const void* src, int count, size_t sizeOfT);

    void* insert(int index, size_t sizeOfT);
    void* insert(int index, const void* src, int count, size_t sizeOfT);

private:
    int safeCountChange(int delta) const;

    std::byte* fStorage{nullptr};
    int fReserve{0};  // size of the allocation in fArray (#elements)
    int fCount{0};    // logical number of elements (fCount <= fReserve)
};

template <typename T> static inline void swap(SkTDStorage& a, SkTDStorage& b) { a.swap(b); }

// SkTDArray<T> implements a std::vector-like array for raw data-only objects that do not require
// construction or destruction. The constructor and destructor for T will not be called; T objects
// will always be moved via raw memcpy. Newly created T objects will contain uninitialized memory.
//
// In most cases, std::vector<T> can provide a similar level of performance for POD objects when
// used with appropriate care. In new code, consider std::vector<T> instead.
template <typename T> class SkTDArray {
public:
    SkTDArray() = default;
    SkTDArray(const T src[], int count) {
        SkASSERT(src || count == 0);
        fStorage.assign(src, count, sizeof(T));
    }
    SkTDArray(const std::initializer_list<T>& list) : SkTDArray(list.begin(), list.size()) {}
    SkTDArray(const SkTDArray<T>& src) {
        fStorage.assign(src.array(), src.count(), sizeof(T));
    }
    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            fStorage.assign(src.data(), src.count(), sizeof(T));
        }
        return *this;
    }

    SkTDArray(SkTDArray<T>&& src)
        : fStorage{std::move(src.fStorage)} {}

    SkTDArray<T>& operator=(SkTDArray<T>&& src) {
        if (this != &src) {
            fStorage = std::move(src.fStorage);
        }
        return *this;
    }

    friend bool operator==(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return a.count() == b.count() &&
               (a.count() == 0 || !memcmp(a.data(), b.data(), SkToSizeT(a.count()) * sizeof(T)));
    }
    friend bool operator!=(const SkTDArray<T>& a, const SkTDArray<T>& b) { return !(a == b); }

    void swap(SkTDArray<T>& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
    }

    bool isEmpty() const { return fStorage.empty(); }
    bool empty() const { return fStorage.empty(); }

    // Return the number of elements in the array
    int    count() const { return fStorage.count(); }
    size_t size() const { return SkToSizeT(fStorage.count()); }

     // Return the total number of elements allocated.
     // reserved() - count() gives you the number of elements you can add
     // without causing an allocation.
    int reserved() const { return fStorage.capacity(); }

    // return the number of bytes in the array: count * sizeof(T)
    size_t bytes() const { return SkToSizeT(this->count()) * sizeof(T); }

    T*       data() { return this->array(); }
    const T* data() const { return this->array(); }
    T*       begin() { return this->array(); }
    const T* begin() const { return this->array(); }
    T*       end() { return this->array() ? this->array() + this->count() : nullptr; }
    const T* end() const { return this->array() ? this->array() + this->count() : nullptr; }

    T& operator[](int index) {
        SkASSERT(index < this->count());
        return this->array()[index];
    }
    const T& operator[](int index) const {
        SkASSERT(index < this->count());
        return this->array()[index];
    }

    T& getAt(int index) { return (*this)[index]; }

    const T& back() const {
        SkASSERT(this->count() > 0);
        return this->array()[this->count() - 1];
    }
    T& back() {
        SkASSERT(this->count() > 0);
        return this->array()[this->count() - 1];
    }

    void reset() {
        fStorage.reset();
    }

    void rewind() {
        fStorage.clear();
    }

     // Sets the number of elements in the array.
     // If the array does not have space for count elements, it will increase
     // the storage allocated to some amount greater than that required.
     // It will never shrink the storage.
    void setCount(int count) {
        fStorage.resize(count, sizeof(T));
    }

    void setReserve(int reserve) {
        this->reserve(SkToSizeT(reserve));
    }
    void reserve(size_t n) {
        fStorage.reserve(n, sizeof(T));
    }

    T* prepend() {
        return reinterpret_cast<T*>(fStorage.prepend(sizeof(T)));
    }

    T* append() {
        return reinterpret_cast<T*>(fStorage.append(sizeof(T)));
    }
    T* append(int count, const T* src = nullptr) {
        return reinterpret_cast<T*>(fStorage.append(src, count, sizeof(T)));
    }

    T* insert(int index) {
        return reinterpret_cast<T*>(fStorage.insert(index, sizeof(T)));
    }
    T* insert(int index, int count, const T* src = nullptr) {
        return reinterpret_cast<T*>(fStorage.insert(index, src, count, sizeof(T)));
    }

    void remove(int index, int count = 1) {
        fStorage.erase(index, count, sizeof(T));
    }

    void removeShuffle(int index) {
        fStorage.replaceWithEndErase(index, sizeof(T));
    }

    int find(const T& elem) const {
        const T* iter = this->begin();
        const T* stop = this->end();

        for (; iter < stop; iter++) {
            if (*iter == elem) {
                return SkToInt(iter - this->array());
            }
        }
        return -1;
    }

    int rfind(const T& elem) const {
        const T* iter = this->end();
        const T* stop = this->begin();

        while (iter > stop) {
            if (*--iter == elem) {
                return SkToInt(iter - stop);
            }
        }
        return -1;
    }

    // Returns true iff the array contains this element.
    bool contains(const T& elem) const { return (this->find(elem) >= 0); }

    // Copies up to max elements into dst. The number of items copied is
    // capped by count - index. The actual number copied is returned.
    int copyRange(T* dst, int index, int max) const {
        SkASSERT(max >= 0);
        SkASSERT(!max || dst);
        if (index >= this->count()) {
            return 0;
        }
        int count = std::min(max, this->count() - index);
        memcpy(dst, this->array() + index, sizeof(T) * count);
        return count;
    }

    void copy(T* dst) const { this->copyRange(dst, 0, this->count()); }

    // routines to treat the array like a stack
    void     push_back(const T& v) { *this->append() = v; }
    T*       push() { return this->append(); }
    const T& top() const { return (*this)[this->count() - 1]; }
    T&       top() { return (*this)[this->count() - 1]; }
    void     pop(T* elem) {
        SkASSERT(this->count() > 0);
        if (elem) *elem = (*this)[this->count() - 1];
        fStorage.decreaseCount();
    }
    void pop() {
        fStorage.decreaseCount();
    }

    void deleteAll() {
        for (auto& p : *this) {
            delete p;
        }
        this->reset();
    }

    void freeAll() {
        for (auto& p : *this) {
            sk_free(p);
        }

        this->reset();
    }

    void unrefAll() {
        for (auto& p : *this) {
            p->unref();
        }
        this->reset();
    }

    void safeUnrefAll() {
        for (auto& p : *this) {
            safeUnref(&p);
        }
        this->reset();
    }

    void shrinkToFit() {
        fStorage.shrinkToFit(sizeof(T));
    }

private:
    T* array() { return fStorage.data<T>(); }
    const T* array() const { return fStorage.data<T>(); }

    // Adjusts the number of elements in the array.
    // This is the same as calling setCount(count() + delta).
    void adjustCount(int delta) {
        SkASSERT(delta > 0);

        // We take care to avoid overflow here.
        // The sum of fCount and delta is at most 4294967294, which fits fine in uint32_t.
        uint32_t count = (uint32_t)this->count() + (uint32_t)delta;
        SkASSERT_RELEASE(SkTFitsIn<int>(count));

        this->setCount(SkTo<int>(count));
    }

    SkTDStorage fStorage;
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) { a.swap(b); }

#endif
