/*
 * Copyright 2020 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"

#include <cinttypes>

class SkString;

enum class ControlCharacterBehavior { kStop, kRemove };
enum class HighBitBehavior { kStop, kReplaceWithToken };

void TransformInputToSkSL(SkSpan<const uint8_t>* data,
                          SkString* sksl,
                          ControlCharacterBehavior controlCharacters,
                          HighBitBehavior highBits);
