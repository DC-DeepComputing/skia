/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkTraceEventCommon.h"

#ifdef SK_USE_PERFETTO_MACRO_OVERRIDES
// This line cannot be placed in a header.
PERFETTO_TRACK_EVENT_STATIC_STORAGE();
#endif
