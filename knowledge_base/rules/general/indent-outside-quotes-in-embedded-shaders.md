---
name: skia/indent-outside-quotes-in-embedded-shaders
description: When embedding multi-line shaders, indent outside quotes and omit newlines in static literals to prevent binary bloat, but retain newlines in dynamic formatting
rubric: safety
severity: should_fix
---

**Rule.** When defining static embedded shader string literals (such as WGSL,
GLSL, or SkSL constexpr char[]), indent outside quotation marks using C++ source
indentation and omit unnecessary newlines ('\n') and indentation spaces to
prevent binary size bloat in .rodata. Conversely, when generating or assembling
dynamic shader fragments via string formatting (such as SkSL::String::printf or
appendf), terminate each statement or logical line with a trailing newline
('\n') rather than concatenating with no whitespace, preserving line structure
for debug logging and compiler error diagnostics.

**Why.** Characters inside string literals are preserved verbatim in .rodata, so
leading spaces and redundant newlines in static shader strings needlessly
inflate binary size without compilation benefit. Conversely, dynamically
formatted shader fragments are assembled at runtime and not stored as bloated
literals; omitting newlines there collapses dumped shaders into a single
unreadable line during debugging and compiler diagnostic reporting.

## Examples

Bad:

```cpp
// Bad: Leading indentation spaces and newlines in static literal bloat .rodata
static constexpr char kShaderText[] =
    "struct VertexOutput {\n"
    "    @builtin(position) position: vec4f,\n"
    "};\n";

// Bad: Stripping newlines in dynamic formatting collapses dumped shader to one line
return SkSL::String::printf(
        "float2x2 vectorXform = float2x2(localToDevice[0].xy, localToDevice[1].xy);"
        "float2 localCoord = tessellate_curve(vectorXform, p01, p23);"
        "stepLocalCoords = localCoord;");
```

Good:

```cpp
// Good: Indented outside quotes with newlines omitted in static literal
static constexpr char kShaderText[] =
    "struct VertexOutput {"
        "@builtin(position) position: vec4f,"
    "};";

// Good: Trailing '\n' per statement preserves shader line readability in debug logs
return SkSL::String::printf(
        "float2x2 vectorXform = float2x2(localToDevice[0].xy, localToDevice[1].xy);\n"
        "float2 localCoord = tessellate_curve(vectorXform, p01, p23);\n"
        "stepLocalCoords = localCoord;\n");
```
