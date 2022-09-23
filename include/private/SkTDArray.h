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

namespace skbase {
int CalculateSizeOrDie(int count, int delta);
} // namespace skbase

class SK_SPI SkTDStorage {
public:
    SkTDStorage() = default;
    SkTDStorage(const SkTDStorage& that) = delete;
    SkTDStorage& operator= (const SkTDStorage& that) = delete;
    SkTDStorage(SkTDStorage&& that);
    SkTDStorage& operator= (SkTDStorage&& that);
    ~SkTDStorage();

    void shrink_to_fit(size_t storageSize);
    int reserve(int newReserve, size_t newStorageSize);

    void swap(SkTDStorage& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
    }

    std::byte* data() { return fStorage; }
    const std::byte* data() const { return fStorage; }

private:
    std::byte* fStorage{nullptr};
};

static inline void swap(SkTDStorage& a, SkTDStorage& b) { a.swap(b); }

class SK_SPI SkSizeMultiplexer {
public:
    explicit SkSizeMultiplexer(int sizeOfT) : fSizeOfT{sizeOfT} {}
    SkSizeMultiplexer(const void* src, int count, int sizeOfT) : fSizeOfT{sizeOfT} {
        SkASSERT(src || count == 0);
        this->assign(src, count);
    }

    // Copy
    SkSizeMultiplexer(const SkSizeMultiplexer& that)
            : SkSizeMultiplexer{that.fStorage, that.fCount, that.fSizeOfT} {}
    SkSizeMultiplexer& operator= (const SkSizeMultiplexer& that) {
        if (this != &that) {
            this->assign(that.fStorage, that.fCount);
        }
        return *this;
    }

    // Move
    SkSizeMultiplexer(SkSizeMultiplexer&& that)
            : fSizeOfT{that.fSizeOfT}
            , fStorage(std::exchange(that.fStorage, nullptr))
            , fReserve{std::exchange(that.fReserve, 0)}
            , fCount{std::exchange(that.fCount, 0)} {}
    SkSizeMultiplexer& operator= (SkSizeMultiplexer&& that) {
        if (this != &that) {
            this->~SkSizeMultiplexer();
            new (this) SkSizeMultiplexer{std::move(that)};
        }
        return *this;
    }

    ~SkSizeMultiplexer() {
        sk_free(fStorage);
    }

    void reset() {
        const int sizeOfT = fSizeOfT;
        this->~SkSizeMultiplexer();
        new (this) SkSizeMultiplexer{sizeOfT};
    }

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

    int capacity() const { return fReserve; }

    void reserve(int newReserve) {
        SkASSERT(newReserve >= 0);
        if (newReserve > fReserve) {
            // Establish the maximum number of elements that includes a valid count for end. In the
            // largest case end() = &fArray[INT_MAX] which is 1 after the last indexable element.
            static constexpr int kMaxCount = INT_MAX;

            // Assume that the array will max out.
            int expandedReserve = kMaxCount;
            if (kMaxCount - newReserve > 4) {
                // Add 1/4 more than we need. Add 4 to ensure this grows by at least 1. Pin to
                // kMaxCount if no room for 1/4 growth.
                int growth = 4 + ((newReserve + 4) >> 2);
                // Read this line as: if (count + growth < kMaxCount) { ... }
                // It's rewritten to avoid signed integer overflow.
                if (kMaxCount - newReserve > growth) {
                    expandedReserve = newReserve + growth;
                }
            }

            fReserve = expandedReserve;
            size_t newStorageSize = this->bytes(fReserve);
            fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, newStorageSize));
        }
    }

    void shrink_to_fit() {
        fReserve = fCount;
        // Because calling realloc with size of 0 is implementation defined, force to a good state
        // by freeing fStorage.
        if (fCount > 0) {
            fStorage = static_cast<std::byte*>(sk_realloc_throw(fStorage, this->bytes(fReserve)));
        } else {
            sk_free(fStorage);
            fStorage = nullptr;
        }
    }

    void swap(SkSizeMultiplexer& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
        swap(fReserve, that.fReserve);
        swap(fCount, that.fCount);
    }

    std::byte* data() { return fStorage; }
    const std::byte* data() const { return fStorage; }

    void* erase(int index, int count) {
        SkASSERT(index >= 0);
        SkASSERT(count >= 0);
        SkASSERT(index + count <= fCount);

        // Check that the resulting size fits in an int. This will abort if not.
        const int newCount = skbase::CalculateSizeOrDie(fCount, -count);

        const int tailBegin = index + count;
        if (count > 0) {
            // Move the tail down if there is one.
            if (fCount > tailBegin) {
                this->internalMove(index, tailBegin, fCount - tailBegin);
            }
            this->resize(newCount);
        }

        return this->address(tailBegin);
    }
    // Removes the entry at 'index' and replaces it with the last array element
    void* removeShuffle(int index) {
        SkASSERT(0 <= index && index < fCount);
        // Check that the new count is valid.
        const int newCount = skbase::CalculateSizeOrDie(fCount, -1);

        // Fill the index if not the last element.
        if (index < newCount) {
            this->internalMove(index, newCount, /*count=*/1);
        }
        this->resize(newCount);
        return this->address(newCount);
    }

    // assign is different from append because it makes sure that the reserved storage fits
    // the data.
    void assign(const void* src, int count) {
        fCount = count;
        this->shrink_to_fit();
        if (count > 0) {
            this->internalCopySrc(/*dstIndex=*/0, src, count);
        }
    }

    void* prepend() {
        return this->insert(/*index=*/0);
    }
    void* append() {
        return this->insert(fCount);
    }
    void* append(const void* src, int count) {
        return this->insert(fCount, src, count);
    }

    void* insert(int index) {
        return this->insert(index, nullptr, /*count=*/1);
    }

    void* insert(int index, const void* src, int count) {
        SkASSERT(0 <= index && index <= fCount);
        SkASSERT(count >= 0);

        if (count > 0) {
            const int oldCount = fCount;

            const int newCount = skbase::CalculateSizeOrDie(fCount, count);
            this->resize(newCount);

            // Shift memory to make space.
            if (oldCount > index) {
                // Safe because index <= oldCount and oldCount + count is safe.
                this->internalMove(index + count, index, oldCount - index);
            }

            if (src != nullptr) {
                this->internalCopySrc(index, src, count);
            }
        }

        return this->address(index);
    }

    void* push_back() {
        if (fCount < fReserve) {
            return this->address(fCount++);
        } else {
            return this->append();
        }
    }

    void pop_back() {
        SkASSERT(fCount > 0);
        fCount--;
    }

    friend bool operator==(const SkSizeMultiplexer& a, const SkSizeMultiplexer& b) {
        return a.size() == b.size() &&
               (a.size() == 0 || !memcmp(a.data(), b.data(), a.bytes(a.size())));
    }
    friend bool operator!=(const SkSizeMultiplexer& a, const SkSizeMultiplexer& b) {
        return !(a == b);
    }

private:
    size_t bytes(int n) const {
        return SkToSizeT(n) * fSizeOfT;
    }
    std::byte* address(int n) {
        return this->data() + this->bytes(n);
    }
    void internalMove(int dstIndex, int srcIndex, int count) {
        this->internalCopySrc(dstIndex, this->address(srcIndex), count);
    }
    void internalCopySrc(int dstIndex, const void* src, int count) {
        SkASSERT(count > 0);
        memmove(this->address(dstIndex), src, this->bytes(count));
    }
    const int fSizeOfT;
    std::byte* fStorage{nullptr};
    int fReserve{0};  // size of the allocation in fArray (#elements)
    int fCount{0};    // logical number of elements (fCount <= fReserve)
};

static inline void swap(SkSizeMultiplexer& a, SkSizeMultiplexer& b) {
    a.swap(b);
}

// SkTDArray<T> implements a std::vector-like array for raw data-only objects that do not require
// construction or destruction. The constructor and destructor for T will not be called; T objects
// will always be moved via raw memcpy. Newly created T objects will contain uninitialized memory.
//
// In most cases, std::vector<T> can provide a similar level of performance for POD objects when
// used with appropriate care. In new code, consider std::vector<T> instead.
template <typename T> class SkTDArray {
public:
    SkTDArray() : fStorage{sizeof(T)} {}
    SkTDArray(const T src[], int count) : fStorage{src, count, sizeof(T)} { }
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
        if (this != &src) {
            fStorage = std::move(src.fStorage);
        }
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
    int count() const { return fStorage.size(); }
    int size()  const { return fStorage.size(); }

     // Return the total number of elements allocated.
     // reserved() - count() gives you the number of elements you can add
     // without causing an allocation.
    int reserved() const { return fStorage.capacity(); }

    // return the number of bytes in the array: count * sizeof(T)
    size_t bytes() const { return fStorage.size_bytes(); }

    T*       data() { return reinterpret_cast<T*>(fStorage.data()); }
    const T* data() const { return reinterpret_cast<const T*>(fStorage.data()); }
    T*       begin() { return this->data(); }
    const T* begin() const { return this->data(); }
    T*       end() { return this->data() ? this->data() + this->size() : nullptr; }
    const T* end() const { return this->data() ? this->data() + this->size() : nullptr; }

    T& operator[](int index) {
        SkASSERT(index < this->size());
        return this->data()[index];
    }
    const T& operator[](int index) const {
        SkASSERT(index < this->size());
        return this->data()[index];
    }

    T& getAt(int index) { return (*this)[index]; }

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

    void rewind() {
        fStorage.clear();
    }

     // Sets the number of elements in the array.
     // If the array does not have space for count elements, it will increase
     // the storage allocated to some amount greater than that required.
     // It will never shrink the storage.
    void setCount(int count) {
        fStorage.resize(count);
    }

    void reserve(size_t n) {
        fStorage.reserve(n);
    }

    T* append() {
        return static_cast<T*>(fStorage.append());
    }
    T* append(int count, const T* src = nullptr) {
        return reinterpret_cast<T*>(fStorage.append(src, count));
    }

    T* insert(int index) {
        return reinterpret_cast<T*>(fStorage.insert(index));
    }
    T* insert(int index, int count, const T* src = nullptr) {
        return reinterpret_cast<T*>(fStorage.insert(index, src, count));
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
    void     push_back(const T& v) { *reinterpret_cast<T*>(fStorage.push_back()) = v; }

    void     pop(T* elem) {
        if (elem) {
            *elem = this->back();
        }
        fStorage.pop_back();
    }
    void pop() {
        fStorage.pop_back();
    }

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

    void shrinkToFit() {
        fStorage.shrink_to_fit();
    }

private:
    SkSizeMultiplexer fStorage;
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) { a.swap(b); }

#endif
