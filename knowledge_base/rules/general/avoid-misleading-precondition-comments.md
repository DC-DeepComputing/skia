---
name: skia/avoid-misleading-precondition-comments
description: When commenting conditional guards, bailouts, or fallback branches, avoid restating predicates, speculating causes, misframing preconditions as bugs, or claiming overly broad invariants
rubric: readability
severity: should_fix
---

**Rule.** Do not write comments that redundantly restate conditional predicates
in prose, mischaracterize optimization fast-path preconditions as unexpected
out-of-sync or corruption bugs, attribute fallback branches to a single
speculative cause, or claim overly broad invariants (such as asserting an object
is not of a base type when it merely failed a more specific subtype or property
check). When documenting early-return checks, bailouts, or fallback paths,
explain the underlying safety invariant or accurately describe the exact
conditions and types that caused the primary path to be bypassed.

**Why.** Restating an if condition adds visual clutter without technical
insight, while describing expected fallback preconditions as bugs misleads
readers into suspecting data corruption. Attributing fallback execution to a
single speculative cause or asserting overly broad exclusions (such as claiming
an object is entirely not of a base type when it only failed a property check)
obscures actual control flow and misleads maintainers about established
preconditions.

## Examples

Bad:

```cpp
// Variant 1: Misframing preconditions as unexpected bugs
SkISize jpegSize = codec->dimensions();
if (jpegSize != imageSize) {
    // We wouldn't expect this to happen given that SkImage should have loaded this correctly,
    // but if they've gotten out of sync, bail out.
    return false;
}

// Variant 2: Attributing fallbacks to a single speculative cause
if (!canvas->drawAsTiledImageRect(image, &src, dst, sampling, p, constraint)) {
    // This is probably a raster-backed canvas - fall back to default draw.
    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

// Variant 3: Claiming overly broad exclusions on fallback
if (existingStmt->is<Block>() && existingStmt->as<Block>().isCompoundStatement()) {
    return append_to_compound_statement(existingStmt, newStmt);
}
// The existing statement was not a Block; create a new compound-statement Block.
return make_compound_statement(existingStmt, newStmt);
```

Good:

```cpp
// Variant 1: State concise safety invariant
SkISize jpegSize = codec->dimensions();
// Safety check: ensure decoded dimensions match before direct embedding.
if (jpegSize != imageSize || !goodColorType || kTopLeft_SkEncodedOrigin != exifOrigin) {
    return false;
}

// Variant 2: Accurately enumerate all conditions triggering fallback
if (!canvas->drawAsTiledImageRect(image, &src, dst, sampling, p, constraint)) {
    // Either the image didn't require tiling or this is a raster-backed
    // canvas. In either case fall back to a default draw.
    canvas->drawImageRect(image, src, dst, sampling, paint, constraint);
}

// Variant 3: Accurately describe the specific condition that failed
if (existingStmt->is<Block>() && existingStmt->as<Block>().isCompoundStatement()) {
    return append_to_compound_statement(existingStmt, newStmt);
}
// The existing statement was not a compound-statement Block; create one.
return make_compound_statement(existingStmt, newStmt);
```
