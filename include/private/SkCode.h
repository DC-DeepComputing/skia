/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCode_DEFINED
#define SkCode_DEFINED

#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include "include/core/SkTypes.h"
#include "include/private/SkSLString.h"

class SkPaint;

enum class Foo {
    kNoneShader,
    kImageShader,
    kGradientShader,
    kBlendShader,

    kBlendMode,

    kLast = kBlendMode
};
static const int kFooBits = 16;
static const uint32_t kFooMask = 0xFFFF;

static const uint32_t kMaxFooSize = 256;

class SkCode {
public:
    static const int kMaxInts = 8;

    int addFoo(Foo foo) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = (int) foo;
        return fNumInts++;
    }

    void patchFoo(int index, Foo foo, uint32_t size) {
        SkASSERT(size < kMaxFooSize);
        SkASSERT(fData[index] == (uint32_t) foo);
        fData[index] = ((int) foo) | (size << kFooBits);
    }

    std::pair<Foo, uint32_t> readFoo(int index) const {
        uint32_t tmpFoo = fData[index] & kFooMask;
        uint32_t size = fData[index] >> kFooBits;

        SkASSERT(size < kMaxFooSize);
        SkASSERT(tmpFoo <= (int) Foo::kLast);
        return { static_cast<Foo>(tmpFoo), size };
    }

    void addInt1(int iFoo) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = iFoo;
        ++fNumInts;
    }

    uint32_t numInts() const { return fNumInts; }

    void dump() const;

    SkSL::String toSkSL() const;

    const void* data() const { return fData.data(); }
    int sizeInBytes() const { return fNumInts * sizeof(uint32_t); }

    bool operator==(const SkCode& rhs) const {
        return fNumInts == rhs.fNumInts &&
               !memcmp(fData.data(), rhs.fData.data(), this->sizeInBytes());
    }

//private:
    int fNumInts = 0;
    std::array<uint32_t, kMaxInts> fData;
};

class SkMagicCodeDictionary {
public:
    struct FullCodeEntry {
        SkCode fCode;
        std::string fSkSL;
    };

    FullCodeEntry* findOrCreateFull(std::unique_ptr<FullCodeEntry>);

private:
    struct Hash {
        size_t operator()(SkCode) const;
    };

    std::unordered_map<SkCode, std::unique_ptr<FullCodeEntry>, Hash> fFullCodeHash;
};

void foo(const SkPaint&);

#endif // SkCode_DEFINED
