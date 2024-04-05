/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlDirectContext.h"
#include "include/gpu/ganesh/mtl/GrMtlBackendContext.h"

#include "metal_context_helper.h"

int main(int argc, char *argv[]) {
    GrMtlBackendContext backendContext = GetMetalContext();
    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeMetal(backendContext);
    if (!ctx) {
        printf("Could not make metal context\n");
        return 1;
    }
    printf("Made the context!?\n");
    return 0;
}
