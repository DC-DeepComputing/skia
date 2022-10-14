/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICC_DEFINED
#define SkICC_DEFINED

#include "include/core/SkData.h"

struct skcms_ICCProfile;
struct skcms_Matrix3x3;
struct skcms_TransferFunction;

SK_API sk_sp<SkData> SkWriteICCProfile(const skcms_TransferFunction&,
                                       const skcms_Matrix3x3& toXYZD50);

SK_API sk_sp<SkData> SkWriteICCProfile(const skcms_ICCProfile*, const char* description);

// Utility function for populating the grid_16 member of skcms_A2B and skcms_B2A
// structures. This converts a floating point XYZD50 point float_xyz to big endian
// u1Fixed15Number values in grid16. It will write 6 bytes.
SK_API void SkICCFloatXYZD50ToGrid16(const float* float_xyz, uint8_t* grid16);

// Utility function for popluating the table_16 member of skcms_Curve structure.
// This converts a float to its representation in table_16. It will write 2 bytes.
SK_API void SkICCFloatToTable16(const float f, uint8_t* table_16);

#endif//SkICC_DEFINED
