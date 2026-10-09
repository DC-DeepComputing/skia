/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_D3D11QueueManager_DEFINED
#define skgpu_graphite_D3D11QueueManager_DEFINED

#include "src/gpu/graphite/QueueManager.h"

#include <d3d11_1.h>
#include <wrl/client.h>

namespace skgpu::graphite {

class D3D11SharedContext;

class D3D11QueueManager final : public QueueManager {
public:
    D3D11QueueManager(ID3D11DeviceContext1* deviceContext, const SharedContext* sharedContext);
    ~D3D11QueueManager() override;

    const D3D11SharedContext* d3d11SharedContext() const;

private:
    std::unique_ptr<CommandBuffer> getNewCommandBuffer(ResourceProvider*, Protected) override;
    OutstandingSubmission onSubmitToGpu(const SubmitInfo&) override;

    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fImmediateDeviceContext;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_D3D11QueueManager_DEFINED
