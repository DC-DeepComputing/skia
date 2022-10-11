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

int CalculateStorageReserve(int newReserve, int sizeOfT);

template <typename T> class SK_SPI SkTDStorage {
public:
    SkTDStorage() = default;
    SkTDStorage(const void* src, int count)
        : fReserve{count}
        , fCount{count} {
        if (count > 0) {
            SkASSERT(src != nullptr);
            size_t storageSize = this->bytes(count);
            fStorage = static_cast<std::byte*>(sk_malloc_throw(storageSize));
            memcpy(fStorage, src, storageSize);
        }
    }

    // Copy
    SkTDStorage(const SkTDStorage<T>& that)
          : SkTDStorage{that.fStorage, that.fCount} {}

    SkTDStorage<T>& operator= (const SkTDStorage<T>& that) {
        if (this != &that) {
            if (that.fCount <= fReserve) {
              fCount = that.fCount;
              if (fCount > 0) {
                memcpy(fStorage, that.data(), that.size_bytes());
              }
            } else {
              *this = SkTDStorage{that.data(), that.size()};
            }
        }
        return *this;
    }

    // Move
    SkTDStorage(SkTDStorage<T>&& that)
        : fStorage(std::exchange(that.fStorage, nullptr))
        , fReserve{that.fReserve}
        , fCount{that.fCount} {}

    SkTDStorage& operator= (SkTDStorage&& that) {
        if (this != &that) {
            this->~SkTDStorage();
            new (this) SkTDStorage{std::move(that)};
        }
        return *this;
    }

    ~SkTDStorage() {
        sk_free(fStorage);
    }

    void reset() {
        this->~SkTDStorage();
        new (this) SkTDStorage<T>{};
    }
    void swap(SkTDStorage<T>& that) {
        std::swap(fStorage, that.fStorage);
        std::swap(fReserve, that.fReserve);
        std::swap(fCount, that.fCount);
    }

    // Size routines
    bool empty() const { return fCount == 0; }
    void clear() { fCount = 0; }
    int size() const { return fCount; }
    void resize(int newCount) {
        SkASSERT(newCount >= 0);
        if (newCount > fReserve) {
            this->reserve(newCount);
        }
        fCount = newCount;
    }
    size_t size_bytes() const { return this->bytes(fCount); }

    // Capacity routines
    int capacity() const { return fReserve; }
    void reserve(int newReserve) {
        SkASSERT(newReserve >= 0);
        if (newReserve > fReserve) {
            fReserve = CalculateStorageReserve(newReserve, SkToInt(sizeof(T)));
            size_t newStorageSize = this->bytes(fReserve);
            fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, newStorageSize));
        }
    }
    void shrink_to_fit() {
        if (fReserve != fCount) {
            fReserve = fCount;
            // Because calling realloc with size of 0 is implementation defined, force to a good state
            // by freeing fStorage.
            if (fReserve > 0) {
                fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, this->bytes(fReserve)));
            } else {
                sk_free(fStorage);
                fStorage = nullptr;
            }
        }
    }

    void* data() { return fStorage; }
    const void* data() const { return fStorage; }

    // Deletion routines
    void erase(int index, int count) {
        SkASSERT(count >= 0);
        SkASSERT(fCount >= count);
        SkASSERT(0 <= index && index <= fCount);

        if (count > 0) {
            // Check that the resulting size fits in an int. This will abort if not.
          const int newCount = this->calculateSizeOrDie(-count);
          this->moveTail(index, index + count, fCount);
          this->resize(newCount);
        }
    }
    // Removes the entry at 'index' and replaces it with the last array element
    void removeShuffle(int index) {
        SkASSERT(fCount > 0);
        SkASSERT(0 <= index && index < fCount);
        // Check that the new count is valid.
        const int newCount = this->calculateSizeOrDie(-1);
        this->moveTail(index, fCount - 1, fCount);
        this->resize(newCount);
    }

    // Insertion routines
    void* prepend() {
        return this->insert(/*index=*/0);
    }

    void append() {
        if (fCount < fReserve) {
            fCount++;
        } else {
          this->insert(fCount);
        }
    }
    void append(int count) {
        SkASSERT(count >= 0);
        // Read as: if (fCount + count <= fReserve) {...}. This is a UB safe way to avoid the add.
        if (fReserve - fCount >= count) {
          fCount += count;
        } else {
          this->insert(fCount, count, nullptr);
        }
    }
    void* append(const void* src, int count) {
        return this->insert(fCount, count, src);
    }

    void* insert(int index) {
        return this->insert(index, /*count=*/1, nullptr);
    }
    void* insert(int index, int count, const void* src) {
        SkASSERT(0 <= index && index <= fCount);
        SkASSERT(count >= 0);

        if (count > 0) {
            const int oldCount = fCount;
            const int newCount = this->calculateSizeOrDie(count);
            this->resize(newCount);
            this->moveTail(index + count, index, oldCount);

            if (src != nullptr) {
                this->copySrc(index, src, count);
            }
        }

        return this->address(index);
    }

    void pop_back() {
        SkASSERT(fCount > 0);
        fCount--;
    }

    template <typename U>
    friend bool operator==(const SkTDStorage<U>& a, const SkTDStorage<U>& b);
    template <typename U>
    friend bool operator!=(const SkTDStorage<U>& a, const SkTDStorage<U>& b);

private:
    size_t bytes(int n) const { return SkToSizeT(n) * SkToSizeT(sizeof(T)); }
    void* address(int n) { return fStorage + this->bytes(n); }

    // Adds delta to fCount. Crash if outside [0, INT_MAX]
    int calculateSizeOrDie(int delta) {
        // Check that count will not go negative.
        SkASSERT_RELEASE(-fCount <= delta);

        // We take care to avoid overflow here.
        // Because count and delta are both signed 32-bit ints, the sum of count and delta is at
        // most 4294967294, which fits fine in uint32_t. Proof follows in assert.
        static_assert(UINT32_MAX >= (uint32_t)INT_MAX + (uint32_t)INT_MAX);
        uint32_t testCount = (uint32_t)fCount + (uint32_t)delta;
        SkASSERT_RELEASE(SkTFitsIn<int>(testCount));
        return SkToInt(testCount);
    }

    // Move the tail of the array defined by the indexes tailStart and tailEnd to dstIndex. The
    // elements at dstIndex are overwritten by the tail.
    void moveTail(int to, int tailStart, int tailEnd) {
        SkASSERT(0 <= to && to <= fCount);
        SkASSERT(0 <= tailStart && tailStart <= tailEnd && tailEnd <= fCount);
        if (to != tailStart && tailStart != tailEnd) {
            this->copySrc(to, this->address(tailStart), tailEnd - tailStart);
        }
    }

    // Copy src into the array at dstIndex.
    void copySrc(int dstIndex, const void* src, int count) {
        SkASSERT(count > 0);
        memmove(this->address(dstIndex), src, this->bytes(count));
    }

    std::byte* fStorage{nullptr};
    int fReserve{0};  // size of the allocation in fArray (#elements)
    int fCount{0};    // logical number of elements (fCount <= fReserve)
};

template <typename T>
bool operator==(const SkTDStorage<T>& a, const SkTDStorage<T>& b) {
    return a.size() == b.size() &&
        (a.size() == 0 || !memcmp(a.data(), b.data(), a.bytes(a.size())));
}

template <typename T>
bool operator!=(const SkTDStorage<T>& a, const SkTDStorage<T>& b) {
    return !(a == b);
}

template <typename T>
static inline void swap(SkTDStorage<T>& a, SkTDStorage<T>& b) {
    a.swap(b);
}

// SkTDArray<T> implements a std::vector-like array for raw data-only objects that do not require
// construction or destruction. The constructor and destructor for T will not be called; T objects
// will always be moved via raw memcpy. Newly created T objects will contain uninitialized memory.
//
// In most cases, std::vector<T> can provide a similar level of performance for POD objects when
// used with appropriate care. In new code, consider std::vector<T> instead.
template <typename T> class SkTDArray  {
public:
    SkTDArray() = default;
    SkTDArray(const T src[], int count) : fStorage{src, count} { }
    SkTDArray(const std::initializer_list<T>& list) : SkTDArray(list.begin(), list.size()) {}

    // Copy
    SkTDArray(const SkTDArray<T>& src) : SkTDArray(src.data(), src.size()) {}
    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        fStorage = src.fStorage;
        return *this;
    }

    // Move
    SkTDArray(SkTDArray<T>&& src) : fStorage{std::move(src.fStorage)} {}
    SkTDArray<T>& operator=(SkTDArray<T>&& src) {
        fStorage = std::move(src.fStorage);
        return *this;
    }

    friend bool operator==(const SkTDArray<T>& a, const SkTDArray<T>& b) {
        return a.fStorage == b.fStorage;
    }
    friend bool operator!=(const SkTDArray<T>& a, const SkTDArray<T>& b) { return !(a == b); }

    void swap(SkTDArray<T>& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
    }

    bool empty() const { return fStorage.empty(); }

    // Return the number of elements in the array
    int size() const { return fStorage.size(); }

    // Return the total number of elements allocated.
    // Note: capacity() - size() gives you the number of elements you can add without causing an
    // allocation.
    int capacity() const { return fStorage.capacity(); }

    // return the number of bytes in the array: count * sizeof(T)
    size_t size_bytes() const { return fStorage.size_bytes(); }

    T*       data() { return static_cast<T*>(fStorage.data()); }
    const T* data() const { return static_cast<const T*>(fStorage.data()); }
    T*       begin() { return this->data(); }
    const T* begin() const { return this->data(); }
    T*       end() { return this->data() + this->size(); }
    const T* end() const { return this->data() + this->size(); }

    T& operator[](int index) {
        SkASSERT(index < this->size());
        return this->data()[index];
    }
    const T& operator[](int index) const {
        SkASSERT(index < this->size());
        return this->data()[index];
    }

    const T& back() const {
        SkASSERT(this->size() > 0);
        return this->data()[this->size() - 1];
    }
    T& back() {
        SkASSERT(this->size() > 0);
        return this->data()[this->size() - 1];
    }

    void reset() {
        fStorage.reset();
    }

    void clear() {
        fStorage.clear();
    }

     // Sets the number of elements in the array.
     // If the array does not have space for count elements, it will increase
     // the storage allocated to some amount greater than that required.
     // It will never shrink the storage.
    void resize(int count) {
        fStorage.resize(count);
    }

    void reserve(size_t n) {
        fStorage.reserve(n);
    }

    T* append() {
        fStorage.append();
        return this->end() - 1;
    }
    T* append(int count) {
        fStorage.append(count);
        return this->end() - count;
    }
    T* append(int count, const T* src) {
        return static_cast<T*>(fStorage.append(src, count));
    }

    T* insert(int index) {
        return static_cast<T*>(fStorage.insert(index));
    }
    T* insert(int index, int count, const T* src = nullptr) {
        return static_cast<T*>(fStorage.insert(index, count, src));
    }

    void remove(int index, int count = 1) {
        fStorage.erase(index, count);
    }

    void removeShuffle(int index) {
        fStorage.removeShuffle(index);
    }

    int find(const T& elem) const {
        const T* iter = this->begin();
        const T* stop = this->end();

        for (; iter < stop; iter++) {
            if (*iter == elem) {
                return SkToInt(iter - this->begin());
            }
        }
        return -1;
    }

    // routines to treat the array like a stack
    void push_back(const T& v) {
        this->append();
        this->back() = v;
    }
    void pop_back() { fStorage.pop_back(); }

    void deleteAll() {
        for (T p : *this) {
            delete p;
        }
        this->reset();
    }

    void freeAll() {
        for (T p : *this) {
            sk_free(p);
        }

        this->reset();
    }

    void unrefAll() {
        for (T p : *this) {
            p->unref();
        }
        this->reset();
    }

    void shrink_to_fit() {
        fStorage.shrink_to_fit();
    }

private:
    SkTDStorage<T> fStorage;
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) { a.swap(b); }

#endif
