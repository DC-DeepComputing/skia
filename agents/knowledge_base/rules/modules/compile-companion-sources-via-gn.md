---
name: skia/modules/compile-companion-sources-via-gn
description: When maintaining or migrating module build scripts, compile companion test and GM sources through GN and Ninja rather than ad-hoc compiler invocations
rubric: correctness
severity: should_fix
---

**Rule.** When migrating or maintaining module build infrastructure, compile all
companion C++ sources (such as test harnesses, GM runners, and resource
decoders) through GN and Ninja rather than invoking compilers (such as em++ or
clang++) directly in custom shell scripts. Ensure companion scripts invoke the
unified build system to produce binaries and link artifacts.

**Why.** Directly compiling C++ sources in shell scripts outside GN and Ninja
causes preprocessor definitions, compiler flags, and ABI attributes (such as
trivial_abi or alignment) to diverge from GN-built static libraries, leading to
header mismatches and One Definition Rule (ODR) or ABI violations.

## Examples

Bad:

```cpp
EMCC_DEBUG=1 ${EMCXX} $RELEASE_CONF -I. -DGR_TEST_UTILS $SKIA_DEFINES \
    tools/Resources.cpp $BASE_DIR/gm_bindings.cpp $BUILD_DIR/libskia.a -o $BUILD_DIR/wasm_gm_tests.js
```

Good:

```cpp
${NINJA} -C ${BUILD_DIR} wasm_gm_tests
```
