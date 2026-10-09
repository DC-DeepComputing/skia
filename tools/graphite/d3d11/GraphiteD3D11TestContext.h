/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skiatest_graphite_D3D11TestContext_DEFINED
#define skiatest_graphite_D3D11TestContext_DEFINED

#include "tools/graphite/GraphiteTestContext.h"
#include "include/gpu/graphite/d3d11/D3D11BackendContext.h"

namespace skiatest::graphite {

class D3D11TestContext : public GraphiteTestContext {
public:
    ~D3D11TestContext() override;

    static std::unique_ptr<GraphiteTestContext> Make();

    skgpu::BackendApi backend() override { return skgpu::BackendApi::kD3D11; }

    skgpu::ContextType contextType() override;

    std::unique_ptr<skgpu::graphite::Context> makeContext(const TestOptions&) override;

    const skgpu::graphite::D3D11BackendContext& getBackendContext() const {
        return fBackendContext;
    }

protected:
    D3D11TestContext(const skgpu::graphite::D3D11BackendContext& backendContext)
            : fBackendContext(backendContext) {}

    skgpu::graphite::D3D11BackendContext fBackendContext;
};

}  // namespace skiatest::graphite

#endif // skiatest_graphite_D3D11TestContext_DEFINED
