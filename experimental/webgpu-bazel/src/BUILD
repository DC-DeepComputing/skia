package(default_visibility = ["//visibility:public"])

DEFAULT_EMSCRIPTEN_LINKOPTS = [
    "-flto",  # Specify lto (has to be set on for compiler as well)
    "--bind",  # Compiles the source code using the Embind bindings to connect C/C++ and JavaScript
    "--closure 1",  # Run the closure compiler
    "-s ALLOW_MEMORY_GROWTH=1",  # Our example doesn't need memory growth
    "-s USE_PTHREADS=0",  # Disable pthreads
    "-s ASSERTIONS=0",  # Turn off assertions
    "-s MODULARIZE=1",
    "-s EXPORT_NAME=WebGPUKitInit",
    "-s DISABLE_EXCEPTION_CATCHING=1",  # Disable all exception catching
    "-s NODEJS_CATCH_EXIT=0",  # We don't have a 'main' so disable exit() catching
    "-s WASM=1",
]

cc_binary(
    name = "hello-world.js",
    srcs = ["bindings.cpp"],
    linkopts = DEFAULT_EMSCRIPTEN_LINKOPTS,
)
