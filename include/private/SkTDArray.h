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

template <size_t SizeOfT>
class SK_SPI SkSizeMultiplexer {
public:
    SkSizeMultiplexer() = default;
    SkSizeMultiplexer(const SkSizeMultiplexer& that) = delete;
    SkSizeMultiplexer& operator= (const SkSizeMultiplexer& that) = delete;
    SkSizeMultiplexer(SkSizeMultiplexer&& that)
            : fStorage(std::move(that.fStorage))
            , fReserve{std::exchange(that.fReserve, 0)}
            , fCount{std::exchange(that.fCount, 0)} {}
    SkSizeMultiplexer& operator= (SkSizeMultiplexer&& that) {
        if (this != &that) {
            this->~SkSizeMultiplexer();
            new (this) SkSizeMultiplexer{std::move(that)};
        }
        return *this;
    }

    void reset() {
        this->~SkSizeMultiplexer();
        new (this) SkSizeMultiplexer{};
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
    size_t size_bytes() const { return Bytes(fCount); }

    int capacity() const { return fReserve; }
    void reserve(int newReserve) {
        SkASSERT(newReserve >= 0);
        if (newReserve > fReserve) {
            fReserve = fStorage.reserve(newReserve, SizeOfT);
        }
    }
    void shrink_to_fit() {
        fStorage.shrink_to_fit(Bytes(fCount));
        fReserve = fCount;
    }

    void swap(SkSizeMultiplexer& that) {
        using std::swap;
        swap(fStorage, that.fStorage);
        swap(fReserve, that.fReserve);
        swap(fCount, that.fCount);
    }

    std::byte* data() { return fStorage.data(); }
    const std::byte* data() const { return fStorage.data(); }

    void* erase(int index, int count) {
        SkASSERT(index >= 0);
        SkASSERT(count >= 0);
        SkASSERT(index + count <= fCount);

        // Check that the resulting size fits in an int. This will abort if not.
        const int newCount = this->calculateSizeDeltaOrDie(-count);

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
        int newCount = this->calculateSizeDeltaOrDie(-1);

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

            const int newCount = this->calculateSizeDeltaOrDie(count);
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
               (a.size() == 0 || !memcmp(a.data(), b.data(), Bytes(a.size())));
    }
    friend bool operator!=(const SkSizeMultiplexer& a, const SkSizeMultiplexer& b) {
        return !(a == b);
    }

private:
    int calculateSizeDeltaOrDie(int delta) const {
        // Check that count will not go negative.
        SkASSERT_RELEASE(-fCount <= delta);

        // We take care to avoid overflow here.
        // Because fCount and delta are both signed 32-bit ints, the sum of fCount and delta is at
        // most 4294967294, which fits fine in uint32_t. Proof follows in assert.
        static_assert(UINT32_MAX >= (uint32_t)INT_MAX + (uint32_t)INT_MAX);
        uint32_t count = (uint32_t)this->size() + (uint32_t)delta;
        SkASSERT_RELEASE(SkTFitsIn<int>(count));
        return SkToInt(count);
    }

    static size_t Bytes(int n) {
        return SkToSizeT(n) * SizeOfT;
    }
    void* address(int n) {
        return fStorage.data() + Bytes(n);
    }
    void internalMove(int dstIndex, int srcIndex, int count) {
        this->internalCopySrc(dstIndex, this->address(srcIndex), count);
    }
    void internalCopySrc(int dstIndex, const void* src, int count) {
        SkASSERT(count > 0);
        memmove(this->address(dstIndex), src, Bytes(count));
    }
    SkTDStorage fStorage;
    int fReserve{0};  // size of the allocation in fArray (#elements)
    int fCount{0};    // logical number of elements (fCount <= fReserve)
};

template <typename T>
static inline void swap(SkSizeMultiplexer<sizeof(T)>& a, SkSizeMultiplexer<sizeof(T)>& b) {
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
    SkTDArray() = default;
    SkTDArray(const T src[], int count) {
        SkASSERT(src || count == 0);
        fStorage.assign(src, count);
    }
    SkTDArray(const std::initializer_list<T>& list) : SkTDArray(list.begin(), list.size()) {}
    SkTDArray(const SkTDArray<T>& src) : SkTDArray(src.data(), src.size()) {}
    SkTDArray<T>& operator=(const SkTDArray<T>& src) {
        if (this != &src) {
            fStorage.assign(src.data(), src.count());
        }
        return *this;
    }

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
    SkSizeMultiplexer<sizeof(T)> fStorage;
};

template <typename T> static inline void swap(SkTDArray<T>& a, SkTDArray<T>& b) { a.swap(b); }

#endif
