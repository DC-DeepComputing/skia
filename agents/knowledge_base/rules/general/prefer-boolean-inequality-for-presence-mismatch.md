---
name: skia/prefer-boolean-inequality-for-presence-mismatch
description: When checking whether exactly one of two nullable pointers or optionals is present, test boolean inequality like !a != !b instead of (a && !b) || (!a && b)
rubric: readability
severity: nit
---

**Rule.** When checking for a presence mismatch between two nullable pointers,
smart pointers, or optional values (where exactly one is present), compare
boolean inequality (such as !a != !b, !!a != !!b, or SkToBool(a) != SkToBool(b))
instead of writing verbose disjunctions like (a && !b) || (!a && b). Evaluate
this presence guard before performing deep comparisons so subsequent code can
safely assume both values are either simultaneously null or non-null.

**Why.** Writing (a && !b) || (!a && b) is verbose, repetitive, and obscures the
intent of checking mutual presence. Comparing boolean inequality (!a != !b or
SkToBool(a) != SkToBool(b)) cleanly expresses the logical XOR mismatch and, when
followed by an early exit, guarantees that subsequent logic can inspect one
operand and assert the presence of the other.

## Examples

Bad:

```cpp
// Verbose disjunction for presence mismatch
if ((this->getEncodedInfo().profile() && !this->dstInfo().colorSpace()) ||
    (!this->getEncodedInfo().profile() && this->dstInfo().colorSpace())) {
    return false;
}
if (this->getEncodedInfo().profile() && this->dstInfo().colorSpace()) {
    skcms_ICCProfile dstProfile;
    this->dstInfo().colorSpace()->toProfile(&dstProfile);
    ...
}
```

Good:

```cpp
// Concise boolean inequality check
if (!!this->getEncodedInfo().profile() != !!this->dstInfo().colorSpace()) {
    return false;
}
if (this->getEncodedInfo().profile()) {
    SkASSERT(this->dstInfo().colorSpace());
    skcms_ICCProfile dstProfile;
    this->dstInfo().colorSpace()->toProfile(&dstProfile);
    ...
}
```
