/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/Recording.h"

#include "src/gpu/graphite/Resource.h"
#include "src/gpu/graphite/TaskGraph.h"

namespace skgpu::graphite {

Recording::Recording(std::unique_ptr<TaskGraph> graph)
        : fGraph(std::move(graph)) {
}

Recording::~Recording() {}


} // namespace skgpu::graphite
