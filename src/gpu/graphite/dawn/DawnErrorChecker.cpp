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
#include "src/gpu/graphite/dawn/DawnSharedContext.h"

namespace skgpu::graphite {
namespace {

constexpr const char* kErrorScopeNames[] = {"validation", "out-of-memory", "internal"};
constexpr DawnErrorType kErrorScopeTypes[] = {
        DawnErrorType::kValidation, DawnErrorType::kOutOfMemory, DawnErrorType::kInternal};
static_assert(std::size(kErrorScopeNames) == std::size(kErrorScopeTypes));
constexpr int kScopeCount = std::size(kErrorScopeTypes);

}  // namespace

DawnErrorChecker::DawnErrorChecker(const DawnSharedContext* sharedContext)
        : fArmed(true), fSharedContext(sharedContext) {
    fSharedContext->device().PushErrorScope(wgpu::ErrorFilter::Validation);
    fSharedContext->device().PushErrorScope(wgpu::ErrorFilter::OutOfMemory);
    fSharedContext->device().PushErrorScope(wgpu::ErrorFilter::Internal);
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

    SkEnumBitMask<DawnErrorType> error = DawnErrorType::kNoError;
    int scopeIdx = kScopeCount - 1;
    DawnAsyncWait wait(fSharedContext);

    auto errorCallback =
            [&](wgpu::PopErrorScopeStatus status, wgpu::ErrorType errorType, const char* msg) {
                if (errorType != wgpu::ErrorType::NoError) {
                    SkASSERT(scopeIdx >= 0);
                    const char* errorScopeName = kErrorScopeNames[scopeIdx];
                    SKGPU_LOG_E("Failed in error scope (%s): %s", errorScopeName, msg);
                    error |= kErrorScopeTypes[scopeIdx];
                }
                scopeIdx--;
                wait.signal();
            };

    // Pop all three error scopes:
    // Internal
    fSharedContext->device().PopErrorScope(wgpu::CallbackMode::AllowSpontaneous, errorCallback);
    wait.busyWait();
    wait.reset();

    // OutOfMemory
    fSharedContext->device().PopErrorScope(wgpu::CallbackMode::AllowSpontaneous, errorCallback);
    wait.busyWait();
    wait.reset();

    // Validation
    fSharedContext->device().PopErrorScope(wgpu::CallbackMode::AllowSpontaneous, errorCallback);
    wait.busyWait();

    fArmed = false;
    return error;
}

}  // namespace skgpu::graphite
