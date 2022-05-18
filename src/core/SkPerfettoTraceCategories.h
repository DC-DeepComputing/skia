/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPerfettoTraceCategories_DEFINED
#define SkPerfettoTraceCategories_DEFINED

#include "third_party/externals/perfetto/sdk/perfetto.h"

 PERFETTO_DEFINE_CATEGORIES(
   perfetto::Category("test").SetDescription("Mockup")
 );

#endif
