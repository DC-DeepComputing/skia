/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "gm/gm.h"

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>


using namespace emscripten;

// Self-documenting types
using JSArray = emscripten::val;
using JSObject = emscripten::val;

/**
 * Returns a JS array of strings containing the names of the registered GMs. GMs are only registered
 * when their source is included in the "link" step, not if they are in something like libgm.a.
 * The names are also logged to the console.
 */
static JSArray ListGMs() {
    SkDebugf("Listing GMs\n");
    JSArray gms = emscripten::val::array();
    for (skiagm::GMFactory fact : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(fact());
        SkDebugf("gm %s\n", gm->getName());
        gms.call<void>("push", std::string(gm->getName()));
    }
    return gms;
}

EMSCRIPTEN_BINDINGS(Skia) {
    function("ListGMs", &ListGMs);
}
