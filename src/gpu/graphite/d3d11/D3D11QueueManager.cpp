/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/d3d11/D3D11QueueManager.h"

#include "src/gpu/graphite/GpuWorkSubmission.h"
#include "src/gpu/graphite/d3d11/D3D11CommandBuffer.h"
#include "src/gpu/graphite/d3d11/D3D11ResourceProvider.h"
#include "src/gpu/graphite/d3d11/D3D11SharedContext.h"

#include <synchapi.h>

namespace skgpu::graphite {

namespace {

class D3D11WorkSubmission final : public GpuWorkSubmission {
public:
    D3D11WorkSubmission(std::unique_ptr<CommandBuffer> cmdBuffer,
                        D3D11QueueManager* queueManager,
                        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> immediateContext,
                        Microsoft::WRL::ComPtr<ID3D11Query> query)
            : GpuWorkSubmission(std::move(cmdBuffer), queueManager)
            , fD3D11QueueManager(queueManager)
            , fImmediateContext(std::move(immediateContext))
            , fQuery(std::move(query))
            , fCompleted(false) {}

    ~D3D11WorkSubmission() override {
        this->recycleQuery();
    }

private:
    void recycleQuery() {
        if (fQuery) {
            fD3D11QueueManager->returnQuery(std::move(fQuery));
            fQuery = nullptr;
        }
    }

    bool onIsFinished(const SharedContext*) override {
        if (fCompleted) {
            return true;
        }
        if (!fQuery) {
            fCompleted = true;
            return true;
        }
        BOOL result = FALSE;
        HRESULT hr = fImmediateContext->GetData(
                fQuery.Get(), &result, sizeof(BOOL), D3D11_ASYNC_GETDATA_DONOTFLUSH);
        if (hr == S_OK) {
            fCompleted = true;
            this->recycleQuery();
            return true;
        }
        return false;
    }

    void onWaitUntilFinished(const SharedContext*) override {
        if (fCompleted) {
            return;
        }
        if (fQuery) {
            BOOL result = FALSE;
            while (fImmediateContext->GetData(fQuery.Get(), &result, sizeof(BOOL), 0) == S_FALSE) {
                Sleep(0);
            }
            this->recycleQuery();
        }
        fCompleted = true;
    }

    D3D11QueueManager* fD3D11QueueManager;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> fImmediateContext;
    Microsoft::WRL::ComPtr<ID3D11Query> fQuery;
    bool fCompleted = false;
};

} // anonymous namespace

D3D11QueueManager::D3D11QueueManager(ID3D11DeviceContext1* deviceContext,
                                     const SharedContext* sharedContext)
        : QueueManager(sharedContext)
        , fImmediateDeviceContext(deviceContext) {}

D3D11QueueManager::~D3D11QueueManager() = default;

const D3D11SharedContext* D3D11QueueManager::d3d11SharedContext() const {
    return static_cast<const D3D11SharedContext*>(fSharedContext);
}

std::unique_ptr<CommandBuffer> D3D11QueueManager::getNewCommandBuffer(
        ResourceProvider* resourceProvider, Protected) {
    return D3D11CommandBuffer::Make(d3d11SharedContext(),
                                    static_cast<D3D11ResourceProvider*>(resourceProvider));
}

Microsoft::WRL::ComPtr<ID3D11Query> D3D11QueueManager::getQuery() {
    if (!fAvailableQueries.empty()) {
        auto query = std::move(fAvailableQueries.back());
        fAvailableQueries.pop_back();
        return query;
    }
    D3D11_QUERY_DESC queryDesc = {};
    queryDesc.Query = D3D11_QUERY_EVENT;
    Microsoft::WRL::ComPtr<ID3D11Query> query;
    HRESULT hr = d3d11SharedContext()->device()->CreateQuery(&queryDesc, &query);
    if (FAILED(hr)) {
        return nullptr;
    }
    return query;
}

void D3D11QueueManager::returnQuery(Microsoft::WRL::ComPtr<ID3D11Query> query) {
    if (query) {
        fAvailableQueries.push_back(std::move(query));
    }
}

QueueManager::OutstandingSubmission D3D11QueueManager::onSubmitToGpu(const SubmitInfo&) {
    SkASSERT(fCurrentCommandBuffer);
    auto* d3d11CmdBuffer = static_cast<D3D11CommandBuffer*>(fCurrentCommandBuffer.get());
    d3d11CmdBuffer->processPendingTextureToBufferCopies();

    Microsoft::WRL::ComPtr<ID3D11Query> query = this->getQuery();
    if (query) {
        fImmediateDeviceContext->End(query.Get());
    }
    fImmediateDeviceContext->Flush();

    return std::make_unique<D3D11WorkSubmission>(
            std::move(fCurrentCommandBuffer), this, fImmediateDeviceContext, std::move(query));
}

} // namespace skgpu::graphite
