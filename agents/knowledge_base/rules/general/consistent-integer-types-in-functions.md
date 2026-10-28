---
name: skia/consistent-integer-types-in-functions
description: When implementing integer loops or functions, match loop counter types to bounds and keep parameters, checks, and casts consistently typed as int or int32_t
rubric: readability
severity: should_fix
---

**Rule.** Maintain consistent integer types across functions and loops instead
of mixing generic int with fixed-width types like int32_t. In functions
operating on plain int, keep signatures, local variables, intermediate range
checks (such as std::numeric_limits<int>::max()), and return casts as int. In
loops, match the iteration variable's type to the limit or bound variable's type
(e.g., use int32_t y when iterating up to an int32_t bound) to avoid mixing
types or triggering comparison warnings.

**Why.** Mixing generic int and fixed-width int32_t creates type confusion,
risks compiler comparison warnings, and falsely implies bit-width guarantees
that are not enforced. Keeping integer types uniform across function bodies,
bounds checks, and loop counters adheres to Skia style and avoids needless
conversions between nominally distinct types.

## Examples

Bad:

```cpp
// Bad: mixing int32_t cast into a function operating on int
int mulInt(int x, int y) {
    int64_t result = (int64_t)x * (int64_t)y;
    if (result > std::numeric_limits<int>::max() || result < std::numeric_limits<int>::min()) {
        fOK = false;
    }
    return (int32_t)result;
}

// Bad: mixing generic int counter with fixed-width int32_t bound
const int32_t height = this->dimensions().height();
for (int y = 0; y < height; ++y) {
    processRow(y);
}
```

Good:

```cpp
// Good: consistent int types throughout signature, checks, and cast
int mulInt(int x, int y) {
    int64_t result = (int64_t)x * (int64_t)y;
    if (result > std::numeric_limits<int>::max() || result < std::numeric_limits<int>::min()) {
        fOK = false;
    }
    return static_cast<int>(result);
}

// Good: loop counter type matches the int32_t bound
const int32_t height = this->dimensions().height();
for (int32_t y = 0; y < height; ++y) {
    processRow(y);
}
```
