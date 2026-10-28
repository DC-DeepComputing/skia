---
name: skia/build-gn/sync-dependency-comments
description: When configuring conditional dependencies or include directories in GN or Bazel, document platform-specific omissions and keep comments synchronized with the active dependency model
rubric: readability
severity: should_fix
---

**Rule.** When declaring or modifying conditional dependencies or header search
paths (include_dirs, deps, public_deps) in GN or Bazel, document platform- or
toolchain-specific branches that omit standard targets or clear include
directories (e.g., when a toolchain SDK like Emscripten bundles its own
headers). Additionally, when refactoring build targets or external library
wiring, update or remove accompanying comments that describe obsolete dependency
models so comments accurately reflect current header origins and target
conditions.

**Why.** Omitting standard dependencies or clearing include paths without
explanation makes platform overrides appear broken or arbitrary to maintainers.
Furthermore, outdated comments mislead developers about where headers or
libraries originate and why conditional target flags exist, obscuring the true
build graph.

## Examples

Bad:

```cpp
# Variant 1: Omitting include_dirs on platform branches without explanation
if (!is_wasm) {
  include_dirs = [ "../externals/dawn/include" ]
} else {
  include_dirs = []
}

# Variant 2: Stale comments contradicting refactored dependency graph
# When building for WASM, WebGPU headers are provided by Emscripten; native builds depend on Dawn.
if (!skia_use_webgpu) {
  if (dawn_enable_d3d12) {
    public_deps += [ "//third_party/externals/dawn/src/dawn:proc" ]
  }
}
```

Good:

```cpp
# Variant 1: Document why toolchain SDK omits standard include paths
if (!is_wasm) {
  include_dirs = [ "../externals/dawn/include" ]
} else {
  # Emscripten bundles its own WebGPU headers.
  include_dirs = []
}

# Variant 2: Synchronized comments matching active dependency model
# Dawn provides different headers depending on whether building for WASM or native.
public_deps += [ "//third_party/externals/dawn/include/dawn:cpp_headers" ]
if (!skia_use_webgpu) {
  if (dawn_enable_d3d12) {
    public_deps += [ "//third_party/externals/dawn/src/dawn:proc" ]
  }
}
```
