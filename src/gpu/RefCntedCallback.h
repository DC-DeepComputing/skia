/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_RefCntedCallback_DEFINED
#define skgpu_RefCntedCallback_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GpuTypes.h"

namespace skgpu {
/**
 * Ref-counted object that calls a callback from its destructor.
 */
class RefCntedCallback : public SkNVRefCnt<RefCntedCallback> {
public:
    using Context = void*;
    using Callback = void (*)(Context);
    using ResultCallback = void (*)(Context, CallbackResult);

    static sk_sp<RefCntedCallback> Make(Callback proc, Context ctx) {
        if (!proc) {
            return nullptr;
        }
        return sk_sp<RefCntedCallback>(new RefCntedCallback(proc, ctx));
    }

    static sk_sp<RefCntedCallback> Make(ResultCallback proc, Context ctx, bool foo) {
        if (!proc) {
            return nullptr;
        }
        return sk_sp<RefCntedCallback>(new RefCntedCallback(proc, ctx, foo));
    }

    ~RefCntedCallback() {
        if (fReleaseProc) {
            SkASSERT(!fResultReleaseProc);
            fReleaseProc(fReleaseCtx);
        } else {
            SkASSERT(fResultReleaseProc);
            fResultReleaseProc(fReleaseCtx, fResult);
        }
    }

    Context context() const { return fReleaseCtx; }

    void setFailureResult() {
        SkASSERT(fResultReleaseProc);
        // Shouldn't really be calling this multiple times.
        SkASSERT(fResult == CallbackResult::kSuccess);
        fResult = CallbackResult::kFailed;
    }

private:
    RefCntedCallback(Callback proc, Context ctx) : fReleaseProc(proc), fReleaseCtx(ctx) {}
    RefCntedCallback(ResultCallback proc, Context ctx, bool)
            : fResultReleaseProc(proc), fReleaseCtx(ctx) {}
    RefCntedCallback(const RefCntedCallback& other)
            : fReleaseProc(other.fReleaseProc)
            , fResultReleaseProc(other.fResultReleaseProc)
            , fReleaseCtx(other.fReleaseCtx)
            , fResult(other.fResult) {
        fIsClone = true;
    }
    RefCntedCallback(RefCntedCallback&&) = delete;
    RefCntedCallback& operator=(const RefCntedCallback&) = delete;
    RefCntedCallback& operator=(RefCntedCallback&&) = delete;

    Callback fReleaseProc = nullptr;
    ResultCallback fResultReleaseProc = nullptr;
    Context fReleaseCtx;
    CallbackResult fResult = CallbackResult::kSuccess;
public:
    bool fIsClone = false;
};

} // namespace skgpu

#endif // skgpu_RefCntedCallback_DEFINED
