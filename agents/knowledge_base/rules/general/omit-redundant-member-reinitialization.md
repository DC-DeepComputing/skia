---
name: skia/omit-redundant-member-reinitialization
description: When default-constructing, value-initializing, or zero-initializing objects or variables, omit redundant .reset(), .clear(), explicit nullptr/zero assignments, or fallback re-initializations
rubric: readability
severity: nit
---

**Rule.** Do not redundantly call .reset() or .clear() on a newly
default-constructed object (such as SkPath), nor reassign default member values
or clear container fields immediately after default-constructing or
value-initializing a struct or state object (such as state = State{}).
Similarly, do not explicitly assign nullptr, false, or 0 to struct fields if the
struct has already been zero-initialized (e.g., via sk_bzero, memset, or
value-initialization), nor re-create or re-assign a default instance in a
fallback else branch when the variable or field was already unconditionally
initialized before the conditional ladder. Either omit the redundant operations,
or replace them with debug assertions (SkASSERT) when verifying state
invariants.

**Why.** Default constructors, value-initialization, zero-initialization
(sk_bzero / memset), and upfront instance creations already initialize objects
and fields to their clean, empty, or default state. Explicitly clearing mutually
exclusive fields or re-creating default instances in fallback branches
introduces dead stores and unnecessary allocations, cluttering code and
misleading readers into thinking prior state might persist.

## Examples

Bad:

```cpp
// Redundant reset on newly default-constructed object
SkPath GrShape::asPath(bool simpleFill) const {
    SkPath out;
    if (!this->isPath() && !this->isArc()) {
        out.reset();
        out.setFillType(kDefaultFillType);
    }
    return out;
}

// Redundant member reinitialization after value-initialization
fDecodingState = DecodingState{};
fDecodingState->fCurrentScanline = 0;
fDecodingState->fFullyDecoded = false;

// Redundant fallback re-initialization after unconditional initialization
this._path = new CanvasKit.PathBuilder();
if (typeof path === 'string') {
    this._path.addPath(CanvasKit.Path.MakeFromSVGString(path));
} else {
    this._path = new CanvasKit.PathBuilder();
}
```

Good:

```cpp
// Newly constructed object is already clean and empty; omit redundant .reset()
SkPath GrShape::asPath(bool simpleFill) const {
    SkPath out;
    if (!this->isPath() && !this->isArc()) {
        out.setFillType(kDefaultFillType);
    }
    return out;
}

// Value-initialization sets in-class defaults; assert invariants if documenting
fDecodingState = DecodingState{};
SkASSERT(!fDecodingState->fFullyDecoded);

// Omit dead fallback else branch since default instance was already initialized upfront
this._path = new CanvasKit.PathBuilder();
if (typeof path === 'string') {
    this._path.addPath(CanvasKit.Path.MakeFromSVGString(path));
}
```
