---
name: skia/sksl/sksl-calculate-findmsb-from-clz
description: When lowering SkSL bit query intrinsics (findMSB, findLSB, bitCount) in backend codegen, compute findMSB from clz and cast unsigned results to signed types
rubric: correctness
severity: should_fix
---

**Rule.** When lowering the findMSB intrinsic using count-leading-zeros
primitives (such as Metal's clz), compute the bit index as (bit_width - 1) -
clz(x) rather than using clz(x) directly, ensuring zero or empty inputs evaluate
to -1. Additionally, when lowering SkSL bit query intrinsics (such as findLSB,
findMSB, or bitCount) to backend shading languages whose built-ins return their
input type (such as Metal's ctz/clz/popcount or WGSL's
firstTrailingBit/firstLeadingBit/countOneBits), explicitly cast unsigned results
to signed integer types so return types match SkSL signed integer semantics.

**Why.** In GLSL and SkSL, bit query intrinsics (bitCount, findLSB, findMSB)
always return signed integers (int/ivec), with -1 representing no set bits for
search operations. In contrast, primitives like clz count leading zeros from the
MSB, and target built-ins like Metal's ctz/clz/popcount or WGSL's
firstTrailingBit/firstLeadingBit/countOneBits return unsigned types matching
their inputs, leading to inverted bit indices or backend shader compilation type
errors if left unadjusted.

## Examples

Bad:

```cpp
// Metal: Using clz directly inverts index order and leaves unsigned result uncast
this->write("select(" + this->typeName(c.type()) + "(clz(" + skTemp + ")), ");

// WGSL: Bit built-ins return unsigned for unsigned inputs, violating SkSL signed return
case k_bitCount_IntrinsicKind:
    return this->assembleSimpleIntrinsic("countOneBits", call);
case k_findLSB_IntrinsicKind:
    return this->assembleSimpleIntrinsic("firstTrailingBit", call);
```

Good:

```cpp
// Metal: Compute (31 - clz) and cast clz result to signed integer type
this->write("select(" + signedType + "(31) - " + signedType + "(clz(" + skTemp + ")), ");

// WGSL: Cast unsigned bit query results back to signed SkSL return type
case k_bitCount_IntrinsicKind: {
    std::string count = this->assembleSimpleIntrinsic("countOneBits", call);
    return arguments[0]->type().componentType().isSigned() ? count :
            (to_wgsl_type(fContext, call.type()) + "(" + count + ")");
}
```
