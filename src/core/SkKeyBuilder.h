/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKeyBuilder_DEFINED
#define SkKeyBuilder_DEFINED

#include "include/core/SkString.h"
#include "include/private/SkTArray.h"

enum class SkBlendMode : int;
enum SkColorType : int;
enum class SkTileMode : int;

class SkKeyBuilder {
public:
    SkKeyBuilder(SkTArray<uint32_t, true>* data) : fData(data) {}

    virtual ~SkKeyBuilder() {
        // Ensure that flush was called before we went out of scope
        SkASSERT(fBitsUsed == 0);
    }

    virtual void addBits(uint32_t numBits, uint32_t val, std::string_view label) {
        SkASSERT(numBits > 0 && numBits <= 32);
        SkASSERT(numBits == 32 || (val < (1u << numBits)));

        fCurValue |= (val << fBitsUsed);
        fBitsUsed += numBits;

        if (fBitsUsed >= 32) {
            // Overflow, start a new working value
            fData->push_back(fCurValue);
            uint32_t excess = fBitsUsed - 32;
            fCurValue = excess ? (val >> (numBits - excess)) : 0;
            fBitsUsed = excess;
        }

        SkASSERT(fCurValue < (1u << fBitsUsed));
    }

    void addBytes(uint32_t numBytes, const void* data, std::string_view label) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
        for (; numBytes --> 0; bytes++) {
            this->addBits(8, *bytes, label);
        }
    }

    bool addBool(bool b, std::string_view label) {
        this->addBits(1, b, label);
        return b;
    }

    void add32(uint32_t v, std::string_view label = "unknown") {
        this->addBits(32, v, label);
    }

    void addColorType(SkColorType ct) {
        constexpr uint32_t kColorTypeBits = 5;
        this->addBits(kColorTypeBits, ct, "colorType");
    }

    void addBlendMode(SkBlendMode bm) {
        constexpr uint32_t kBlendModeBits = 5;
        this->addBits(kBlendModeBits, static_cast<uint32_t>(bm), "blendMode");
    }

    void addTileMode(SkTileMode tm) {
        constexpr uint32_t kTileModeBits = 2;
        this->addBits(kTileModeBits, static_cast<uint32_t>(tm), "tileMode");
    }

    virtual void appendComment(const char* comment) {}

    // Introduces a word-boundary in the key. Must be called before using the key with any cache,
    // but can also be called to create a break between generic data and backend-specific data.
    void flush() {
        if (fBitsUsed) {
            fData->push_back(fCurValue);
            fCurValue = 0;
            fBitsUsed = 0;
        }
    }

private:
    SkTArray<uint32_t, true>* fData;
    uint32_t fCurValue = 0;
    uint32_t fBitsUsed = 0;  // ... in current value
};

class SkStringKeyBuilder : public SkKeyBuilder {
public:
    SkStringKeyBuilder(SkTArray<uint32_t, true>* data) : SkKeyBuilder(data) {}

    void addBits(uint32_t numBits, uint32_t val, std::string_view label) override {
        SkKeyBuilder::addBits(numBits, val, label);
        fDescription.appendf("%.*s: %u\n", (int)label.size(), label.data(), val);
    }

    void appendComment(const char* comment) override {
        fDescription.appendf("%s\n", comment);
    }

    SkString description() const { return fDescription; }

private:
    SkString fDescription;
};

#endif // SkKeyBuilder_DEFINED
