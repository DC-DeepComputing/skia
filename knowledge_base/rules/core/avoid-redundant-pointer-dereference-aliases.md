---
name: skia/core/avoid-redundant-pointer-dereference-aliases
description: When accessing members through a pointer or smart pointer, use direct arrow notation instead of introducing redundant local pointer-dereference reference aliases
rubric: readability
severity: nit
---

**Rule.** Do not introduce or retain unnecessary local reference aliases (such
as `const Type& alias = *ptr;`) solely to dereference a pointer or smart pointer
when direct member access via arrow notation (`ptr->member`) is clear and
concise. Access members directly through the pointer rather than creating a
local dereferenced alias, especially after refactoring removes the primary need
to query the underlying object.

**Why.** Creating local reference aliases that merely dereference a pointer adds
visual noise and extra named variables without providing meaningful abstraction.
Furthermore, retaining obsolete local aliases after refactoring can obscure
object ownership boundaries and tempt callers to inspect internal state that
should remain encapsulated.

## Examples

Bad:

```cpp
// Redundant local reference alias to dereference a pointer
const SkPathData& pdata = *src.fPathData;

this->addRaw(pdata.raw(src.getFillType(), SkResolveConvexity::kYes));
fLastMoveIndex = SkPathPriv::FindLastMoveToIndex(fVerbs, fPts.size());
fType = pdata.fType;
fIsA  = pdata.fIsA;
```

Good:

```cpp
// Direct arrow member access through the pointer
this->addRaw(src.fPathData->raw(src.getFillType(), SkResolveConvexity::kYes));
fLastMoveIndex = SkPathPriv::FindLastMoveToIndex(fVerbs, fPts.size());
fType = src.fPathData->fType;
fIsA  = src.fPathData->fIsA;
```
