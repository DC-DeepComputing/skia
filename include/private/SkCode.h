/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCode_DEFINED
#define SkCode_DEFINED

#include <array>
#include "include/core/SkTypes.h"



class SkCode {
public:
    static const int kMaxInts = 8;

    void addInt(int iFoo) {
        SkASSERT(fNumInts < kMaxInts);
        fInts[fNumInts] = iFoo;
        ++fNumInts;
    }

    int numInts() const { return fNumInts; }

    void dump() const;

//private:
    int fNumInts = 0;
    std::array<int, kMaxInts> fInts;
};

#endif // SkCode_DEFINED
