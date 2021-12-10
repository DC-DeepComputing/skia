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

class SkPaint;

static const int kBlendShader = 1;

class SkCode {
public:
    static const int kMaxInts = 8;

    void addInt(int iFoo) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = iFoo;
        ++fNumInts;
    }

    int numInts() const { return fNumInts; }

    void dump() const;

    std::string toSkSL() const;

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
