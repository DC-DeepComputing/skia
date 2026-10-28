---
name: skia/when-defining-sksl-shader-code-via-concatenated-string-liter
description: When defining SkSL shader code via concatenated string literals, omit newline escape sequences across statements separated by syntax delimiters
rubric: readability
severity: nit
---

**Rule.** Do not include explicit newline escape sequences ('\n') in
concatenated C++ string literals defining SkSL shader code; SkSL syntax
delimiters (such as semicolons and braces) already separate statements.

**Why.** When defining multi-line SkSL shaders via concatenated C++ string
literals, SkSL statements and declarations are already syntactically delimited
by semicolons and braces. Explicit '\n' characters inside the string fragments
are redundant and add unnecessary visual clutter.

## Examples

Bad:

```cpp
static constexpr char gAgtmSKSL[] =
    "uniform shader curve_xym;\n"
    "uniform half weight_i;\n"
    "uniform half4 mix_rgbx_i;\n";
```

Good:

```cpp
static constexpr char gAgtmSKSL[] =
    "uniform shader curve_xym;"
    "uniform half weight_i;"
    "uniform half4 mix_rgbx_i;";
```
