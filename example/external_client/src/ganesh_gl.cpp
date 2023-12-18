/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/GrDirectContext.h"

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.png>\n", argv[0]);
        return 1;
    }

    GrContextOptions opts;
    opts.fSuppressPrints = true;
    sk_sp<const GrGLInterface> interface = GrGLMakeNativeInterface(); FIXME
    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeGL(interface, opts);

    if (!ctx) {
        printf("Could not make GL interface\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    return 0;
}
