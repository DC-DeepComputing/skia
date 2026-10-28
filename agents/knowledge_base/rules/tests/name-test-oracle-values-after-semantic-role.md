---
name: skia/tests/name-test-oracle-values-after-semantic-role
description: When declaring test oracle or reference values in unit tests, name variables after their semantic role (e.g., expectedClip) rather than cryptic abbreviations or type suffixes
rubric: readability
severity: nit
---

**Rule.** When declaring variables for test oracle outputs, reference algorithm
results, or baseline expectations in unit tests, name them after their semantic
role using an expected<Concept> prefix (such as expectedClip or expectedClipLb).
Avoid cryptic algorithm acronyms, terse shorthand, or container type-tag
suffixes (such as lbOpt).

**Why.** Cryptic abbreviations and type tags like lbOpt encode low-level
implementation mechanics and container wrappers rather than the test value's
semantic role. Descriptive names like expectedClipLb clarify test intent and
make assertions, comparison failures, and debugging straightforward.

## Examples

Bad:

```cpp
// Bad: Encodes algorithm acronym and container type rather than semantic role
auto lbOpt = reference_clip(line, tileMinX, tileMinY, tileMaxX, tileMaxY);
if (lbOpt.has_value()) {
    matchesLb = matchPt(result[i], (*lbOpt)[i]);
}
```

Good:

```cpp
// Good: Semantically identifies the expected value and reference algorithm
auto expectedClipLb = reference_clip(line, tileMinX, tileMinY, tileMaxX, tileMaxY);
if (expectedClipLb.has_value()) {
    matchesLb = matchPt(result[i], (*expectedClipLb)[i]);
}
```
