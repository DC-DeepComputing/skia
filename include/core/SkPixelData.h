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
class SkPixelData : public SkRefCnt {
public:
    SkPixelData();

    enum Type {
        kTextureProxy,
        kPixelRef,
    };

    uint32_t getId() const { return fID; }
    virtual Type type() const = 0;

private:
    uint32_t fID;

    static uint32_t NextId();
};

#endif
