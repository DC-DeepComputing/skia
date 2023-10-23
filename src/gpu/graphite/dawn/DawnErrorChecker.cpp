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

constexpr const char* kErrorScopeNames[] = {"validation", "out-of-memory", "internal"};
constexpr DawnErrorType kErrorScopeTypes[] = {
        DawnErrorType::kValidation, DawnErrorType::kOutOfMemory, DawnErrorType::kInternal};
static_assert(std::size(kErrorScopeNames) == std::size(kErrorScopeTypes));
constexpr int kScopeCount = std::size(kErrorScopeTypes);

}  // namespace

DawnErrorChecker::DawnErrorChecker(const wgpu::Device& device,
                                   SkEnumBitMask<DawnErrorType> fatalErrors)
        : fArmed(true), fDevice(device), fFatalErrors(fatalErrors) {
    fDevice.PushErrorScope(wgpu::ErrorFilter::Validation);
    fDevice.PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    fDevice.PushErrorScope(wgpu::ErrorFilter::Internal);
}

DawnErrorChecker::~DawnErrorChecker() {
    [[maybe_unused]] auto err = this->popErrorScopes();
    SkASSERT(!fArmed);
    SkASSERT(err == DawnErrorType::kNoError);
}

SkEnumBitMask<DawnErrorType> DawnErrorChecker::popErrorScopes() {
    if (!fArmed) {
        return DawnErrorType::kNoError;
    }

    struct ErrorState {
        SkEnumBitMask<DawnErrorType> fError;
        const SkEnumBitMask<DawnErrorType> fFatalErrors;
        int fScopeIdx;
        DawnAsyncWait fWait;

        ErrorState(const wgpu::Device& device, SkEnumBitMask<DawnErrorType> fatalErrors)
                : fError(DawnErrorType::kNoError)
                , fScopeIdx(kScopeCount - 1)
                , fWait(device)
                , fFatalErrors(fatalErrors) {}
    } errorState(fDevice, fFatalErrors);

    wgpu::ErrorCallback errorCallback = [](WGPUErrorType status, const char* msg, void* userData) {
        ErrorState* errorState = static_cast<ErrorState*>(userData);
        if (status != WGPUErrorType_NoError) {
            SkASSERT(errorState->fScopeIdx >= 0);
            const char* errorScopeName = kErrorScopeNames[errorState->fScopeIdx];
            const DawnErrorType errorType = kErrorScopeTypes[errorState->fScopeIdx];

            if (errorState->fFatalErrors & errorType) {
                SKGPU_LOG_F("Failed in error scope (%s): %s", errorScopeName, msg);
            }
            else {
                SKGPU_LOG_E("Failed in error scope (%s): %s", errorScopeName, msg);
            }
            errorState->fError |= errorType;
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
    return errorState.fError;
}

}  // namespace skgpu::graphite
