---
name: skia/sksl/prefer-is-same-expression-tree
description: When checking whether two SkSL Expression trees are equivalent in compiler passes, call Analysis::IsSameExpressionTree instead of manually comparing node kinds, types, and member fields
rubric: readability
severity: should_fix
---

**Rule.** When comparing two SkSL Expression trees for equivalence in compiler
passes, optimizations, or IR constructors, call
Analysis::IsSameExpressionTree(*a, *b) instead of writing manual kind checks,
type matches, or member comparisons. Rely on this standardized recursive helper
to compare expressions (such as variables, literals, constructors, swizzles, and
prefixes) and verify type compatibility.

**Why.** Analysis::IsSameExpressionTree provides a complete, standardized
recursive comparison for SkSL expression nodes and their types. Open-coding
ad-hoc kind checks (such as checking is<VariableReference>() and comparing
variable pointers) is brittle, duplicates AST traversal boilerplate, and easily
misses equivalent subexpressions or edge cases.

## Examples

Bad:

```cpp
if (!leftExpr->type().matches((*rightExpr)->type())) {
    return false;
}
SkASSERT(leftExpr->is<VariableReference>());
if (leftExpr->as<VariableReference>().variable() !=
    (*rightExpr)->as<VariableReference>().variable()) {
    return false;
}
```

Good:

```cpp
if (!Analysis::IsSameExpressionTree(*leftExpr, **rightExpr)) {
    return false;
}
```
