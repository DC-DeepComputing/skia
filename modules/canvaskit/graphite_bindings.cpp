/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#include "modules/canvaskit/WasmCommon.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/dawn/DawnBackendContext.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>


EMSCRIPTEN_BINDINGS(Graphite) {
    constant("graphite", true);

    function("TestGraphite", optional_override([]()->bool {
        std::unique_ptr<skgpu::graphite::Context> context =
        skgpu::graphite::ContextFactory::MakeDawn({}, {});

        return !!context.get();
    }));

    //     std::unique_ptr<skgpu::graphite::Context> context =
    //    skgpu::graphite::ContextFactory::MakeMetal(backendContext, options);
}
