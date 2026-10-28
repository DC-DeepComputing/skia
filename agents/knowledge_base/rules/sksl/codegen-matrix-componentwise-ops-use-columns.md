---
name: skia/sksl/codegen-matrix-componentwise-ops-use-columns
description: When synthesizing or polyfilling component-wise matrix operations in shader code generators, emit operations across column vectors rather than decomposing matrices into scalar elements
rubric: safety
severity: should_fix
---

**Rule.** When synthesizing or polyfilling component-wise matrix operations
(such as matrixCompMult or component-wise binary matrix arithmetic) in shader
code generators, emit operations over column vectors rather than decomposing the
matrix into individual scalar elements. Take advantage of target shading
language support (such as in WGSL and MSL) for vector operators and SIMD
instructions across column vectors.

**Why.** In shading languages like WGSL and MSL, matrices are column-major
structures composed of column vectors, and vector arithmetic operators operate
component-wise. Emitting operations over column vectors produces significantly
more concise shader code and allows downstream shader compilers and GPU vector
ALUs to utilize SIMD vector operations directly rather than relying on
scalarization or vectorization passes.

## Examples

Bad:

```cpp
int columns = arguments[0]->type().columns();
int rows = arguments[0]->type().rows();
for (int c = 0; c < columns; ++c) {
    for (int r = 0; r < rows; ++r) {
        String::appendf(&expr, "%s%s[%d][%d] * %s[%d][%d]",
                        separator().c_str(), arg0.c_str(), c, r, arg1.c_str(), c, r);
    }
}
```

Good:

```cpp
int columns = arguments[0]->type().columns();
for (int c = 0; c < columns; ++c) {
    String::appendf(&expr, "%s%s[%d] * %s[%d]",
                    separator().c_str(), arg0.c_str(), c, arg1.c_str(), c);
}
```
