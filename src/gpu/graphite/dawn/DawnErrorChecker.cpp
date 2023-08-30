/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/dawn/DawnErrorChecker.h"

#include "include/private/base/SkAssert.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/dawn/DawnAsyncWait.h"

namespace skgpu::graphite {
namespace {

const char* kErrorScopeNames[] = {"validation", "out-of-memory", "internal"};
constexpr int kScopeCount = 3;

}  // namespace

DawnErrorChecker::DawnErrorChecker(const wgpu::Device& device) : fArmed(true), fDevice(device) {
    fDevice.PushErrorScope(wgpu::ErrorFilter::Validation);
    fDevice.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    fDevice.PushErrorScope(wgpu::ErrorFilter::Internal);
}

DawnErrorChecker::~DawnErrorChecker() {
    [[maybe_unused]] bool success = this->check();
    SkASSERT(!fArmed);
    SkASSERT(success);
}

bool DawnErrorChecker::check() {
    if (!fArmed) {
        return true;
    }

    struct ErrorState {
        bool fFailed;
        int fScopeIdx;
        DawnAsyncWait fWait;

        ErrorState(const wgpu::Device& device)
                : fFailed(false), fScopeIdx(kScopeCount - 1), fWait(device) {}
    } errorState(fDevice);

    auto errorCallback = [](WGPUErrorType status, const char* msg, void* userData) {
        ErrorState* errorState = static_cast<ErrorState*>(userData);
        if (status != WGPUErrorType_NoError) {
            SkASSERT(errorState->fScopeIdx >= 0);
            SKGPU_LOG_E(
                    "Failed in error scope (%s): %s", kErrorScopeNames[errorState->fScopeIdx], msg);
            errorState->fFailed = true;
        }
        errorState->fScopeIdx--;
        errorState->fWait.signal();
    };

    // Pop all three error scopes:
    // Internal
    fDevice.PopErrorScope(errorCallback, &errorState);
    errorState.fWait.busyWait();
    errorState.fWait.reset();

    // OutOfMemory
    fDevice.PopErrorScope(errorCallback, &errorState);
    errorState.fWait.busyWait();
    errorState.fWait.reset();

    // Validation
    fDevice.PopErrorScope(errorCallback, &errorState);
    errorState.fWait.busyWait();

    fArmed = false;
    return !errorState.fFailed;
}

}  // namespace skgpu::graphite
