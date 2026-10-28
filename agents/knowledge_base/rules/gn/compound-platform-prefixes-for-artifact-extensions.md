---
name: skia/gn/compound-platform-prefixes-for-artifact-extensions
description: When configuring platform- or target-specific artifact extensions in build toolchains, use compound extensions with leading platform qualifiers instead of trailing suffixes
rubric: readability
severity: should_fix
---

**Rule.** When configuring platform- or target-specific file output extensions
in build toolchains or targets, use compound extensions that place the platform
qualifier before the standard file extension (e.g., '.wasm.a' or '.wasm.so')
rather than appending it after the extension (e.g., '.a-wasm' or '.so-wasm').
Keep the standard, recognizable file extension at the end of the filename.

**Why.** Build systems, linters, package managers, and external tools identify
file formats by their terminal extension (such as .a for static archives or .so
for shared libraries). Appending a platform qualifier as a suffix breaks
standard file type identification, whereas placing the platform qualifier as a
prefix preserves the terminal extension while distinguishing target-specific
artifacts.

## Examples

Bad:

```cpp
if (current_os == "wasm" && !is_canvaskit) {
  default_output_extension = ".a-wasm"
}
```

Good:

```cpp
if (current_os == "wasm" && !is_canvaskit) {
  default_output_extension = ".wasm.a"
}
```
