load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

BASE_LINKOPTS = [
    #"-flto",  # https://github.com/emscripten-core/emsdk/issues/807
    "--bind",  # Compiles the source code using the Embind bindings to connect C/C++ and JavaScript
    "-s ALLOW_MEMORY_GROWTH=1",
    "-s USE_PTHREADS=0",  # Disable pthreads
    "-s MODULARIZE=1",
    "-s EXPORT_NAME=InitWasmGMTests",
    "-s DISABLE_EXCEPTION_CATCHING=1",  # Disable all exception catching
    "-s NODEJS_CATCH_EXIT=0",  # We don't have a 'main' so disable exit() catching
    "-s WASM=1",
    "-s MAX_WEBGL_VERSION=2",
]

RELEASE_OPTS = [
    "--closure 1",  # Run the closure compiler
    "-s ASSERTIONS=0",  # Turn off assertions
]

DEBUG_OPTS = [
    "--closure 0",  # Do not use closure
    "-s ASSERTIONS=1",  # Turn on assertions
    "-s GL_ASSERTIONS=1",
]

config_setting(
    name = "release_opts",
    values = {"compilation_mode": "opt"},
)

config_setting(
    name = "debug_opts",
    values = {"compilation_mode": "dbg"},
)

cc_library(
    name = "skia-core",
    srcs = [
        "//include:private-hdrs",
        "//include:third-party-skcms-hdrs",
        "//include:third-party-vulkan-hdrs",
        "//src:codec-hdrs",
        "//src:core-hdrs",
        "//src:core-srcs",
        "//src:effects-hdrs",
        "//src:effects-srcs",
        "//src:gpu-hdrs",
        "//src:gpu-srcs",
        "//src:image-hdrs",
        "//src:image-srcs",
        "//src:images-hdrs",
        "//src:images-srcs",
        "//src:opts-hdrs",
        "//src:pathops-hdrs",
        "//src:pathops-srcs",
        "//src:sfnt-hdrs",
        "//src:sfnt-srcs",
        "//src:shaders-hdrs",
        "//src:shaders-srcs",
        "//src:sksl-hdrs",
        "//src:sksl-srcs",
        "//src:utils-hdrs",
        "//src:utils-srcs",
        "//third_party:libjpeg-turbo",
        "//third_party:libjpeg-turbo-hdrs",
        "//third_party:skcms",
    ],
    hdrs = [
        "//include:codec-hdrs",
        "//include:config-hdrs",
        "//include:core-hdrs",
        "//include:effects-hdrs",
        "//include:encode-hdrs",
        "//include:gpu-hdrs",
        "//include:pathops-hdrs",
        "//include:ports-hdrs",
        "//include:sksl-hdrs",
        "//include:utils-hdrs",
    ],
    copts = ["-Ithird_party/externals/libjpeg-turbo/"],
    textual_hdrs = ["//src:sksl-txts"],
)

cc_library(
    name = "gms",
    srcs = [
        "//gm:gms",
        "//gm:srcs",
        "//src/ports:empty_factory",
        "//src/ports:malloc",
        "//src/ports:stdio",
        "//tools:hdrs",
        "//tools:srcs",
    ],
    hdrs = [
        "//gm:hdrs",
    ],
    textual_hdrs = ["//tools:txts"],
    deps = [":skia-core"],
)

cc_binary(
    name = "gm-bindings",
    srcs = [
        "gm_bindings.cpp",
        "//gm:gms",  # Required for the registry to work
    ],
    linkopts = select({
        ":debug_opts": BASE_LINKOPTS + DEBUG_OPTS,
        ":release_opts": BASE_LINKOPTS + RELEASE_OPTS,
        "//conditions:default": BASE_LINKOPTS + RELEASE_OPTS,
    }),
    # This target won't build successfully on its own because of missing emscripten
    # headers etc. Therefore, we hide it from wildcards.
    tags = ["manual"],
    deps = [":gms"],
)

wasm_cc_binary(
    name = "gm-bindings-wasm",
    cc_target = ":gm-bindings",
)
