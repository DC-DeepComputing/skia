---
name: skia/omit-self-evident-code-comments
description: When writing function bodies, methods, or switch cases, omit tautological or self-evident comments and express preconditions directly with assertions
rubric: readability
severity: nit
---

**Rule.** Do not add comments to functions, methods, switch cases, or dispatch
tables that merely narrate self-evident code operations (such as noting that a
flag or feature is being enabled) or state tautological facts (such as
explaining that a null pointer indicates absence). Reserve comments for subtle
rationale, edge cases, or non-obvious invariants, and express preconditions
directly in code using assertions (such as SkASSERT) rather than prose comments.

**Why.** Restating self-explanatory code or tautological facts line-by-line adds
visual clutter and cognitive overhead without technical insight, and can mislead
readers into suspecting hidden contracts. Enforcing preconditions directly with
SkASSERT catches bugs in debug builds and validates contracts, while comments
remain focused on non-obvious architectural rationale.

## Examples

Bad:

```cpp
// Case 1: Tautological comment stating the obvious
// image == nullptr indicates there is no image.
void installImage(void* imageData) {
    this->fImage = imageData;
}

// Case 2: Redundant function narration
// Update backend label to match fLabel and mark clean.
void synchronizeBackendLabel() {
    if (!fLabel.empty()) {
        this->setBackendLabel(("Skia_" + fLabel).c_str());
    }
    fBackendLabelDirty = false;
}

// Case 3: Redundant inline comment in switch case
case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT: {
    chain(newChainEnd, pNext);
    // Enable the main feature
    if (exts.fFrameBoundaryEXT) {
        features->frameBoundary = VK_TRUE;
    }
    break;
}
```

Good:

```cpp
// Case 1: Precondition enforced via assertion
void installImage(void* imageData) {
    SkASSERT(!this->setImageHasBeenCalled());
    fImage = imageData;
}

// Case 2: Self-explanatory code without narrative comment
void synchronizeBackendLabel() {
    if (!fLabel.empty()) {
        this->setBackendLabel(("Skia_" + fLabel).c_str());
    }
    fBackendLabelDirty = false;
}

// Case 3: Uniform switch case without redundant comment
case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT: {
    chain(newChainEnd, pNext);
    if (exts.fFrameBoundaryEXT) {
        features->frameBoundary = VK_TRUE;
    }
    break;
}
```
