---
name: skia/gpu/prefer-explicit-invalid-sentinel-factory
description: When passing, returning, or initializing invalid Graphite IDs, keys, or descriptors, call explicit Invalid() sentinel factories rather than empty braces {} or static_casting raw primitives
rubric: readability
severity: nit
---

**Rule.** When passing, returning, or initializing sentinel or invalid states
for Graphite types that provide explicit sentinel factories (such as
UniquePaintParamsID, PaintParamsKey, Transform, or UploadInstance), call
Type::Invalid(). Do not use empty braced initialization {} or default
construction, and do not static_cast raw primitive constants (such as
SK_InvalidUniqueID) to obtain an invalid instance. Avoid relying on empty braces
{} paired with inline parameter comments to convey unassigned or invalid state
at call sites.

**Why.** Graphite ID, key, and transform types encapsulate their underlying
representation and define static Invalid() methods as the canonical
representation for sentinel, unassigned, or non-shading states (such as
depth-only clip draws). Calling Type::Invalid() communicates intent
unambiguously at call sites, preserves class encapsulation, and prevents
confusing empty braces with value-initialized defaults or resorting to
non-idiomatic casts of raw primitive constants.

## Examples

Bad:

```cpp
// Ambiguous empty initializer with inline parameter comment
fDC->recordDraw(renderer, Transform::Identity(), Geometry(Shape(devicePath)), clip, order,
                /*paintID*/{}, /*dstReadReq*/false, /*usesAdvancedBlend*/false,
                fDataGatherer.get(), /*stroke*/nullptr);

// Casting raw primitive constant instead of using typed sentinel factory
UniquePaintParamsID paintID = keyResult.has_value()
        ? std::get<0>(*keyResult)
        : static_cast<UniquePaintParamsID>(SK_InvalidUniqueID);

// Returning unassigned key via bare empty braces
return {};
```

Good:

```cpp
// Explicit sentinel factory clarifies intent at call site
fDC->recordDraw(renderer, Transform::Identity(), Geometry(Shape(devicePath)), clip, order,
                UniquePaintParamsID::Invalid(), DstUsage::kNone,
                fDataGatherer.get(), /*stroke*/nullptr);

// Calling typed sentinel factory method directly
UniquePaintParamsID paintID = keyResult.has_value()
        ? std::get<0>(*keyResult)
        : UniquePaintParamsID::Invalid();

// Returning canonical invalid key
return PaintParamsKey::Invalid();
```
