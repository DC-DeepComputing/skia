---
name: skia/derive-x-macro-strings-from-tokens
description: 'When defining X-macros or list macros that map identifiers to name strings, stringify identifier tokens with #Token rather than passing redundant string literals'
rubric: readability
severity: should_fix
---

**Rule.** In X-macros or list macros that generate enums and string name
mappings, derive string representations directly from identifier tokens using
the preprocessor stringizing operator (#Token) in consumer macro expansions
rather than passing separate string literal arguments in the macro list. Only
pass explicit string literals when the string representation differs
intentionally from the token name.

**Why.** Passing redundant string literals alongside identifier tokens in macro
tables creates maintenance overhead and risks casing, spelling, or formatting
drift between the enum and its name. Using the preprocessor stringizing operator
(#Token) guarantees exact consistency between the identifier and its string
representation.

## Examples

Bad:

```cpp
#define SKGPU_RENDERSTEP_TYPES(M1, M3) \
    M1(Invalid)                        \
    M3(TessellateWedges, EvenOdd, "evenodd")

#define CASE3(BaseName, VariantName, VariantTag) \
    case RenderStepID::k##BaseName##_##VariantName: return #BaseName "RenderStep[" VariantTag "]";
```

Good:

```cpp
#define SKGPU_RENDERSTEP_TYPES(M1, M2) \
    M1(Invalid)                        \
    M2(TessellateWedges, EvenOdd)

#define CASE2(BaseName, VariantName) \
    case RenderStepID::k##BaseName##_##VariantName: return #BaseName "RenderStep[" #VariantName "]";
```
