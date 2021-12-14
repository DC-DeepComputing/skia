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

enum class CodeSnippetID {
    kNoneShader,
    kImageShader,
    kGradientShader,
    kBlendShader,     // for now just driven by an SkBlendMode

    kSimpleBlendMode,   // an actual SkBlendMode

    kLast = kSimpleBlendMode
};
static constexpr int kCodeSnippetIDCount = static_cast<int>(CodeSnippetID::kLast) + 1;
static const int kCodeSnippetIDBits = 16;
static const uint32_t kCodeSnippetIDMask = 0xFFFF;

static const uint32_t kMaxCodeSnippetSize = 256;

CodeSnippetID ExtractCodeSnippetID(uint32_t code);

class SkCode {
public:
    static const int kMaxInts = 8;

    int addFoo(CodeSnippetID foo) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = (int) foo;
        return fNumInts++;
    }

    void patchFoo(int index, CodeSnippetID id, uint32_t size) {
        SkASSERT(size < kMaxCodeSnippetSize);
        SkASSERT(fData[index] == (uint32_t) id);
        fData[index] = ((int) id) | (size << kCodeSnippetIDBits);
    }

    static std::pair<CodeSnippetID, uint32_t> ReadCodeSnippetID(uint32_t header) {
        CodeSnippetID id = ExtractCodeSnippetID(header);
        uint32_t size = header >> kCodeSnippetIDBits;

        SkASSERT(size < kMaxCodeSnippetSize);
        return { id, size };
    }

    void addInt1(int iFoo) {
        SkASSERT(fNumInts < kMaxInts);
        fData[fNumInts] = iFoo;
        ++fNumInts;
    }

    uint32_t numInts() const { return fNumInts; }

    static uint32_t DumpFrom(const uint32_t* data);
    void dump() const;

    static uint32_t ToSkSLFrom(const uint32_t* data, SkSL::String* result);
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
        SkCode       fCode;
        SkSL::String fSkSL;
    };

    const FullCodeEntry* findOrCreateFull(std::unique_ptr<FullCodeEntry>);

    const FullCodeEntry* exhaustiveSearch(const SkSL::String& code) const;

private:
    struct Hash {
        size_t operator()(SkCode) const;
    };

    std::unordered_map<SkCode, std::unique_ptr<FullCodeEntry>, Hash> fFullCodeHash;
};

void AddCodeToDictionary(const SkPaint&);
void CheckCodeHasBeenSeen(const SkSL::String&);

#endif // SkCode_DEFINED
