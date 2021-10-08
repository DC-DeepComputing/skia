/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <emscripten/bind.h>

using namespace emscripten;

class Greet {
    public:

    static std::string SayHello(const std::string &name) {
        return "Yo! " + name;
    };
};

EMSCRIPTEN_BINDINGS(Skia) {
    emscripten::class_<Greet>("Greet")
        .constructor<>()
        .class_function("SayHello", &Greet::SayHello);
}
