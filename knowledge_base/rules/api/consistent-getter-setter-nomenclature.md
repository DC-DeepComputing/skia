---
name: skia/api/consistent-getter-setter-nomenclature
description: When defining class accessors, avoid bare action verbs, use canonical get/set prefixes or consistent noun phrases, align nomenclature symmetrically, and match backing fields
rubric: readability
severity: should_fix
---

**Rule.** Maintain consistent naming across class accessors, member variables,
and constructor parameters. Follow canonical get<Property>() and set<Property>()
prefixes for property accessors per Skia style, and avoid bare imperative verb
phrases for getters (e.g., replaceTabCharacters()) that read like mutating or
action operations. Keep getter conventions uniform within each class without
mixing get-prefixed accessors (e.g., getSegmentMask()) with bare noun phrases
(e.g., points()), pair getters and setters symmetrically using identical
property nomenclature (e.g., getResourceCacheLimit() with
setResourceCacheLimit(), maxBudgetedBytes() with setMaxBudgetedBytes()), spell
out descriptive names instead of opaque acronyms (e.g., floatStorageManager()
over fsm()), and align member fields ('f' prefixed), getters, and constructor
parameters.

**Why.** Mismatched terminology or omitting standard get/set prefixes makes APIs
unintuitive to discover and inconsistent with Skia style conventions. Using bare
verb phrases for getters causes them to be mistaken for mutating or action
operations rather than accessors. Mixing get-prefixes with bare noun phrases
creates an unpredictable interface, while opaque acronyms and divergent
parameter or field names obscure caller semantics and cause terminology drift.

## Examples

Bad:

```cpp
// Bare imperative verb phrase on getter reads like an action/mutating method
bool replaceTabCharacters() const { return fReplaceTabCharacters; }
void setReplaceTabCharacters(bool value) { fReplaceTabCharacters = value; }

// Divergent getter and setter nomenclature
size_t maxBudgetedBytes() const;
void setMaxGpuCacheBudget(size_t bytes);

// Omitting get prefix on get/set accessors or mixing getter styles in one class
bool applyRoundingHack() const { return fApplyRoundingHack; }
SkSpan<const SkPoint> points() const { return fPoints; }
uint8_t getSegmentMask() const { return fSegmentMask; }

// Opaque acronym in getter name
FloatStorageManager* fsm() const { return fFloatStorageManager; }

// Divergent member/getter name from constructor parameter
PaintParams::PaintParams(bool dstReadRequired)
    : fReadsDst(dstReadRequired) {}

// Legacy accessor name contradicting refactored type and field
DstReadStrategy dstReadRequirement() const { return fPipelineInfo.fDstReadStrategy; }
```

Good:

```cpp
// Prefixing verb-like getter with get to pair symmetrically with setter
bool getReplaceTabCharacters() const { return fReplaceTabCharacters; }
void setReplaceTabCharacters(bool value) { fReplaceTabCharacters = value; }

// Matching getter and setter nomenclature with canonical get/set prefixes
bool getApplyRoundingHack() const { return fApplyRoundingHack; }
void setApplyRoundingHack(bool applyRoundingHack) { fApplyRoundingHack = applyRoundingHack; }

// Symmetrical setter without get prefix where bare nouns are used
size_t maxBudgetedBytes() const;
void setMaxBudgetedBytes(size_t bytes);

// Uniform getter naming within the class
SkSpan<const SkPoint> points() const { return fPoints; }
uint8_t segmentMask() const { return fSegmentMask; }

// Descriptive getter name matching sibling accessors
FloatStorageManager* floatStorageManager() const { return fFloatStorageManager; }

// Aligned constructor parameter, member variable, and getter
PaintParams::PaintParams(bool dstReadRequired)
    : fDstReadRequired(dstReadRequired) {}
bool dstReadRequired() const { return fDstReadRequired; }

// Accessor name aligned with refactored type and backing field
DstReadStrategy dstReadStrategy() const { return fPipelineInfo.fDstReadStrategy; }
```
