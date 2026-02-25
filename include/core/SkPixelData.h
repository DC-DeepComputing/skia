/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelData_DEFINED
#define SkPixelData_DEFINED

#include <cstdint>
#include "include/core/SkRefCnt.h"

/**
 * SkPixelData is the class representing the storage block for pixel data. It serves as the abstract
 * source of pixel memory—the 2D array of data—that is read from and written to by
 * higher-level Skia objects like SkImage and SkSurface.
 */
//TODO: consider different names before submitting.
class SkPixelData : public SkRefCnt {
public:
    SkPixelData() : fId(this->getId()) {}

    enum Type {
        kTextureProxy,
        kPixelRef,
    };

    // TODO: decide if we should use a new unified global pool for IDs and ask reviewers what
    // they think about it.
    uint32_t getId();
    virtual Type type() const = 0;

private:
    uint32_t fId;
};

#endif
