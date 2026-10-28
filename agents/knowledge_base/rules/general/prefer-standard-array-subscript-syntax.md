---
name: skia/prefer-standard-array-subscript-syntax
description: When indexing arrays or raw pointers, use standard array[index] subscript notation rather than inverted index[array] syntax permitted by pointer commutativity
rubric: readability
severity: nit
---

**Rule.** When indexing arrays or raw pointers, always use standard array[index]
subscript notation rather than inverted index[array] syntax. Never place the
array identifier or pointer expression inside the square brackets.

**Why.** In C and C++, array subscripting E1[E2] is defined as *((E1) + (E2)),
which makes pointer and integer operands commutative. However, inverted
subscripting like index[array] or (mask)[table] is unintuitive, violates
standard coding idioms, and obscures which operand is the container and which is
the index.

## Examples

Bad:

```cpp
return (width & 0xF)[width_for_usWidth];
```

Good:

```cpp
return width_for_usWidth[width & 0xF];
```
