---
name: skia/assert-return-types-in-ts-declaration-tests
description: In TypeScript declaration tests, assert expression and method return types using // $ExpectType <Type> instead of calling methods without type verification
rubric: testing
severity: should_fix
---

**Rule.** In TypeScript type-definition and declaration tests (such as dtslint
suites), assert the return type of tested functions and methods using `//
$ExpectType <Type>`. Do not invoke methods without verifying their returned type
or assign return values to untyped variables without a type expectation.

**Why.** Invoking a method or function in TypeScript declaration tests only
verifies parameter compatibility and argument types. Adding `// $ExpectType
<Type>` validates that the declared return type in the `.d.ts` file matches
expectations, catching unintended `any`, `void`, or mismatched return types.

## Examples

Bad:

```cpp
// Invoking methods or assigning results without asserting the return type
const l = mAnim.setTransform('foo', [1, 2], [3, 4], [5, 6], 90, 1, 0);
mAnim.setColor('slider', CK.WHITE);
```

Good:

```cpp
// Asserting the exact expected return type with $ExpectType
const l = mAnim.setTransform('foo', [1, 2], [3, 4], [5, 6], 90, 1, 0); // $ExpectType boolean
const res = mAnim.setColor('slider', CK.WHITE); // $ExpectType boolean
```
