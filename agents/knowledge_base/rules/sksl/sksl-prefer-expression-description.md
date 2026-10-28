---
name: skia/sksl/sksl-prefer-expression-description
description: When formatting an SkSL Expression for display, call expr->description() rather than downcasting to VariableReference, and avoid passing tighter operator precedences like kPostfix
rubric: readability
severity: should_fix
---

**Rule.** When obtaining the string representation, identifier, or variable name
of an SkSL Expression (or IRNode), call expr->description() rather than manually
downcasting to a concrete subclass like VariableReference (e.g.,
expr->as<VariableReference>().variable()->name()). Furthermore, when formatting
expressions for standalone display (such as in diagnostics or logging), use the
zero-argument description() helper (or pass OperatorPrecedence::kExpression)
rather than a tighter operator precedence like kPostfix, to avoid injecting
unnecessary parentheses around subexpressions.

**Why.** Downcasting to VariableReference assumes the expression is always a
bare variable reference, which crashes or asserts when given other valid
expressions like FieldAccess. Calling description() handles formatting
polymorphically, and using top-level expression precedence ensures
subexpressions are not wrapped in superfluous parentheses.

## Examples

Bad:

```cpp
// Bad: Downcasting assumes the expression is always a bare variable
std::string name = std::string(uniformExpr->as<VariableReference>().variable()->name());
mangledName += "_" + name;

// Bad: Passing tighter operator precedence causes unnecessary parentheses in standalone output
return fieldAccess ? fieldAccess->description(OperatorPrecedence::kPostfix)
                   : std::string(var->name());
```

Good:

```cpp
// Good: Rely on description() to format expressions polymorphically
mangledName += "_" + uniformExpr->description();

// Good: Use zero-argument description() or kExpression for standalone expression formatting
return fieldAccess ? fieldAccess->description()
                   : std::string(var->name());
```
