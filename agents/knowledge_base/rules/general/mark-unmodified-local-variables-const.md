---
name: skia/mark-unmodified-local-variables-const
description: When declaring unmutated local variables, compile-time constants, loop-invariant flags, or test fixtures, initialize them at point of use and qualify as const or constexpr
rubric: readability
severity: should_fix
---

**Rule.** Declare local variables at their point of initialization rather than
declaring them beforehand with placeholder values, and qualify them with const
or constexpr (preferring constexpr when values are known at compile time, such
as numeric limits or compile-time calculations) whenever they are not
reassigned. This applies to boolean status, return results, dimensions,
transformation matrices and geometry objects (such as SkM44 or SkMatrix),
configuration flags, as well as immutable local test data, containers (such as
std::vector or SkTDArray), and expected-result collections passed to
verification helpers.

**Why.** Declaring and initializing local variables at their point of use
minimizes scope, eliminates dummy or uninitialized states, and clarifies data
flow. Marking unmutated local variables, matrices, and collections const or
constexpr prevents accidental modification or reassignment, communicates
read-only intent, enables compile-time evaluation where possible, and helps both
human readers and compilers recognize invariant values.

## Examples

Bad:

```cpp
// Unmodified local matrix and compile-time constants left mutable
SkM44 negativeScale = SkM44::Scale(-SK_Scalar1, SK_Scalar1);
int maxInt = std::numeric_limits<int>::max();
int sqrtMax = 46340;

// Separate prior declaration with placeholder initialization
bool got_chrm = false;
got_chrm = reader.try_get_chrm(wx, wy, rx, ry, gx, gy, bx, by);

// Unmodified local test data and expected collections left mutable
SkTDArray<Tile> expected = {
        Tile(1, 24, 0, B),
};
check_tiles_match(reporter, lines, expected);
```

Good:

```cpp
// Const- or constexpr-qualified immutable local variables and constants
const SkM44 negativeScale = SkM44::Scale(-SK_Scalar1, SK_Scalar1);
constexpr int maxInt = std::numeric_limits<int>::max();
constexpr int sqrtMax = 46340;

// Direct const initialization at point of use
const bool got_chrm = reader.try_get_chrm(wx, wy, rx, ry, gx, gy, bx, by);

// Const-qualified local test data and expected collections
const SkTDArray<Tile> expected = {
        Tile(1, 24, 0, B),
};
check_tiles_match(reporter, lines, expected);
```
