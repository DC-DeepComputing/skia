---
name: skia/sksl/cover-nested-expressions-in-codegen-tests
description: When implementing or modifying shader code generation or AST expression lowering, include tests exercising nested and chained expressions to verify recursion and scoping
rubric: testing
severity: should_fix
---

**Rule.** When implementing or modifying code generation or AST lowering for an
expression construct (especially constructs that hoist preparatory statements,
rewrite into control flow, or allocate temporary scratch variables like WGSL
ternary expressions), add test shaders covering nested and chained expression
variations. Verify that nested subexpressions in all operand positions (such as
true, false, and condition branches of ternary operators) correctly evaluate
recursively without variable shadowing, scope leakage, or broken evaluation
order across backends.

**Why.** Expressions that lower into statement sequences or temporary scratch
variables often succeed on flat, top-level expressions but fail when nested
inside other expressions. Nested and chained expressions stress-test recursive
code generation, ensuring scratch variables have correct lifetimes, outer scopes
do not shadow inner variables, and subexpressions execute in the required order.

## Examples

Bad:

```cpp
// resources/sksl/shared/TernaryExpression.sksl
// Tests cover only flat, top-level ternary expressions:
half4 result = (colorWhite != colorBlue) ? colorRed : colorGreen;
return result;
```

Good:

```cpp
// resources/sksl/shared/TernaryNesting.sksl
// Tests explicitly exercise nested and chained expressions in both branches:
half4 result = (colorWhite != colorBlue)
                 ? (colorGreen == colorRed ? colorRed : colorGreen)
                 : (colorRed != colorGreen ? colorBlue : colorWhite);
return colorRed == colorBlue  ? colorWhite :
       colorRed != colorGreen ? result :
       colorRed == colorWhite ? colorBlue :
                                colorRed;
```
