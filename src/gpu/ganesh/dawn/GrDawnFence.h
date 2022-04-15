/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnFence_DEFINED
#define GrDawnFence_DEFINED

#include "webgpu/webgpu_cpp.h"

#include <atomic>

// Utility for monitoring the execution of an asynchronous Dawn-API event.
class GrDawnAsyncWait final {
public:
    explicit GrDawnAsyncWait(const wgpu::Device& device);

    // Returns true if the wait has been signaled and false otherwise. This function yields
    // execution to the event loop where Dawn's asynchronous tasks get scheduled and returns
    // as soon as the loop yields the execution back to the caller.
    bool yieldAndCheck() const;

    // Busy-waits until this wait has been signaled.
    // TODO(armansito): This could benefit from a timeout in the case the wait never gets signaled.
    void busyWait() const;

    // Marks this wait as resolved. Once called, all calls to `check` and `block` will
    // return true immediately.
    void signal() { fSignaled.store(true); }

private:
    wgpu::Device fDevice;
    std::atomic_bool fSignaled;
};

// A Fence abstraction driven by the WebGPU `Queue.onSubmittedWorkDone()` event
// (https://www.w3.org/TR/webgpu/#dom-gpuqueue-onsubmittedworkdone).
class GrDawnFence final {
public:
    explicit GrDawnFence(const wgpu::Device& device);

    // Busy-waits until the `Queue.onSubmittedWorkDone()` event is received.
    void busyWait() const { fWait.busyWait(); }

    // Returns the underlying wait.
    const GrDawnAsyncWait& wait() const { return fWait; }

private:
    GrDawnAsyncWait fWait;
};

#endif  // GrDawnFence_DEFINED
