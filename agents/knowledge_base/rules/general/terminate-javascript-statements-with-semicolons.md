---
name: skia/terminate-javascript-statements-with-semicolons
description: When writing JavaScript, TypeScript, or HTML scripts, terminate statements with an explicit semicolon instead of relying on automatic semicolon insertion
rubric: readability
severity: nit
---

**Rule.** In JavaScript, TypeScript, and HTML script blocks, terminate
statements (such as `return`, `throw`, `break`, `continue`, variable
declarations, and expression statements) with an explicit semicolon. Do not omit
trailing semicolons or rely on Automatic Semicolon Insertion (ASI).

**Why.** JavaScript's Automatic Semicolon Insertion (ASI) has subtle edge cases
where newlines are parsed as continuous expressions or cause unintended early
returns. Explicit statement-terminating semicolons eliminate grammatical
ambiguity, prevent ASI bugs, and maintain codebase consistency.

## Examples

Bad:

```cpp
if (scratch[0] === -1) {
    return null
}
const result = computeValue()
```

Good:

```cpp
if (scratch[0] === -1) {
    return null;
}
const result = computeValue();
```
