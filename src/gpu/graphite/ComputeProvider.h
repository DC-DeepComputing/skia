/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputeProvider_DEFINED
#define skgpu_graphite_ComputeProvider_DEFINED

#include <memory>

namespace skgpu::graphite {

class Caps;

class ComputeProvider {
public:
    ~ComputeProvider();

private:
    friend class Context; // for constructor access

    ComputeProvider(const Caps* caps);

    ComputeProvider(const ComputeProvider&) = delete;
    ComputeProvider(ComputeProvider&&) = delete;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_ComputeProvider_DEFINED
