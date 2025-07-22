/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#include "modules/canvaskit/WasmCommon.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>


EMSCRIPTEN_BINDINGS(Graphite) {
    constant("graphite", true);

    //     std::unique_ptr<skgpu::graphite::Context> context =
    //    skgpu::graphite::ContextFactory::MakeMetal(backendContext, options);
}