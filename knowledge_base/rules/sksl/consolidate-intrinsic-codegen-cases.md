---
name: skia/sksl/consolidate-intrinsic-codegen-cases
description: In SkSL backend code generators, deduplicate intrinsic emission by consolidating identical cases with [[fallthrough]] or factoring shared call prefixes into helper methods
rubric: design
severity: should_fix
---

**Rule.** In SkSL backend code generators, avoid duplicating dispatch logic or
argument assembly across related intrinsic kinds. When intrinsic variants share
identical dispatch and formatting (such as min, max, clamp), consolidate switch
cases with [[fallthrough]] and query function names directly from the
FunctionCall AST node (e.g., c.function().mangledName()). When related intrinsic
families share common parameter preparation or call prefixes (such as texture
sampling variants separating texture/sampler pairs or projecting coordinates),
factor the shared call prefix into a helper method rather than duplicating
argument assembly in each case.

**Why.** Duplicating dispatch boilerplate and argument formatting across
intrinsic kinds bloats code generation backends and risks divergence when
argument translation logic changes. Consolidating identical cases and extracting
shared call prefixes keeps switch cases concise and isolates variant-specific
parameters.

## Examples

Bad:

```cpp
// Case 1: Duplicating identical dispatch across related intrinsics
case k_clamp_IntrinsicKind:
    if (fCaps.fVectorClampMinMaxSupport || !c.type().isVector()) {
        this->writeSimpleIntrinsic(c);
    } else {
        this->writeScalarizedIntrinsicCall("clamp", c);
    }
    return true;
case k_min_IntrinsicKind:
    if (fCaps.fVectorClampMinMaxSupport || !c.type().isVector()) {
        this->writeSimpleIntrinsic(c);
    } else {
        this->writeScalarizedIntrinsicCall("min", c);
    }
    return true;

// Case 2: Duplicating shared call prefix and argument splitting
case k_sampleLod_IntrinsicKind: {
    std::string coords = this->assembleSampleCoordinates(*arguments[1]);
    return "textureSampleLevel(" +
           this->assembleExpression(*arguments[0], Precedence::kSequence) + kTextureSuffix + ", " +
           this->assembleExpression(*arguments[0], Precedence::kSequence) + kSamplerSuffix + ", " +
           coords + ", " +
           this->assembleExpression(*arguments[2], Precedence::kSequence) + ')';
}
```

Good:

```cpp
// Case 1: Consolidate cases and extract function name from AST
case k_min_IntrinsicKind:
    [[fallthrough]];
case k_max_IntrinsicKind:
    [[fallthrough]];
case k_clamp_IntrinsicKind: {
    if (fCaps.fVectorClampMinMaxSupport || !c.type().isVector()) {
        this->writeSimpleIntrinsic(c);
    } else {
        this->writeScalarizedIntrinsicCall(c);  // queries c.function().mangledName()
    }
    return true;
}

// Case 2: Factor shared call prefix into a helper method
case k_sampleLod_IntrinsicKind: {
    return this->assemblePartialSampleCall("textureSampleLevel",
                                           *arguments[0],
                                           *arguments[1]) + ", " +
           this->assembleExpression(*arguments[2], Precedence::kSequence) + ')';
}
```
