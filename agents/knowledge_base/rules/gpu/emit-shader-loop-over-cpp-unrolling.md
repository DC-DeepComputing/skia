---
name: skia/gpu/emit-shader-loop-over-cpp-unrolling
description: When generating shaders with fixed iteration counts, emit constant-bounded shader for-loops instead of manually unrolling statements via C++ string concatenation
rubric: readability
severity: should_fix
---

**Rule.** When generating shader code (such as WGSL, SkSL, or GLSL) with
parameterized iteration counts, emit a shader for-loop with a compile-time
constant bound rather than manually unrolling loop iterations via C++ string
concatenation. Rely on downstream shader compilers (such as Tint) and GPU driver
optimizers to unroll constant-bounded loops automatically.

**Why.** Manually unrolling loop bodies via C++ string concatenation complicates
shader generator code and bloats templates. Modern shader compilers and GPU
drivers can optimize and unroll constant-bounded loops without manual string
assembly in C++.

## Examples

Bad:

```cpp
std::ostringstream ss;
ss << "var sum = textureLoad(colorMap, coords, 0);\n";
for (int i = 1; i < sampleCount; ++i) {
    ss << "sum += textureLoad(colorMap, coords, " << i << ");\n";
}
```

Good:

```cpp
static constexpr char kReadMSAAFSTemplate[] =
    "const sampleCount = %u;\n"
    "var sum = vec4f(0.0);\n"
    "for (var i: u32 = 0; i < sampleCount; i = i + 1) {\n"
    "    sum += textureLoad(colorMap, coords, i);\n"
    "}\n"
    "return sum * (1.0 / f32(sampleCount));\n";
return SkStringPrintf(kReadMSAAFSTemplate, sampleCount);
```
