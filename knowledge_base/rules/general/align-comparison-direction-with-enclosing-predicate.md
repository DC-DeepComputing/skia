---
name: skia/align-comparison-direction-with-enclosing-predicate
description: When writing relational comparisons inside predicate functions, orient the expression direction to match the function's linguistic framing rather than inverting operands
rubric: readability
severity: nit
---

**Rule.** Orient comparison expressions to match the linguistic framing of the
enclosing function name or predicate (e.g., write `subject < threshold` rather
than `threshold > subject` when implementing a predicate named `...IsBelow` or
testing if a value is below a limit). When testing whether a quantity exceeds,
falls below, or reaches a boundary, place the evaluated subject on the side
indicated by the predicate name so the relational operator aligns with reader
expectation.

**Why.** Inverting operands and relational operators (such as checking
`threshold > subject` inside a function named `...IsBelow`) requires readers to
mentally flip the expression to verify correctness. Aligning the comparison
direction with the predicate's framing makes algorithmic intent immediately
clear and reduces cognitive overhead.

## Examples

Bad:

```cpp
bool StreamRemainingLengthIsBelow(SkStream* stream, size_t len) {
    size_t remainingBytes = stream->getLength() - stream->getPosition();
    // Anti-pattern: inverted operand order and operator direction
    return len > remainingBytes;
}
```

Good:

```cpp
bool StreamRemainingLengthIsBelow(SkStream* stream, size_t len) {
    size_t remainingBytes = stream->getLength() - stream->getPosition();
    // Good: subject is compared against threshold matching the function name
    return remainingBytes < len;
}
```
