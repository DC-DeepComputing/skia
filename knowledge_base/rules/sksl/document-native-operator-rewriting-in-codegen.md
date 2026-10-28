---
name: skia/sksl/document-native-operator-rewriting-in-codegen
description: When lowering native target-language operators into conditional statements in shader code generators, document why native operators cannot be used and what side effects require guarding
rubric: readability
severity: nit
---

**Rule.** In shader backend code generators and AST lowering passes, when
rewriting an expression into conditional control flow or scratch blocks despite
the target language natively supporting the operator (such as short-circuiting
`&&` and `||`), document why the native operator cannot be used. Specifically
explain when subexpression evaluation emits hoisted preparatory statements that
must be conditionally guarded to preserve evaluation semantics, and clarify
under what conditions the native operator can safely remain.

**Why.** Target languages like WGSL often support short-circuiting boolean
operators natively, making lowering passes that rewrite them into
multi-statement conditional blocks appear redundant or confusing. Documenting
target-language limitations (such as lack of expression-level assignments) and
the necessity of guarding hoisted preparatory statements from premature
execution prevents maintainers from erroneously removing or simplifying the
transformation.

## Examples

Bad:

```cpp
// If the operator is && or ||, we need to handle short-circuiting properly.
std::string expr;
if (op.kind() == OperatorKind::LOGICALAND && Analysis::HasSideEffects(right)) {
    // Converts `left_expression && right_expression` into the following block:
    ...
}
```

Good:

```cpp
// If the operator is && or ||, we need to handle short-circuiting properly. Specifically, we
// sometimes need to emit extra statements to paper over functionality that WGSL lacks, like
// assignment in the middle of an expression. We need to guard those extra statements, to ensure
// that they don't occur if the expression evaluation is short-circuited. Converting the
// expression into an if-else block keeps the short-circuit property intact even when extra
// statements are involved.
// If the RHS doesn't have any side effects, then it's safe to just leave the expression as-is,
// since we know that any possible extra statements are non-side-effecting.
std::string expr;
if (op.kind() == OperatorKind::LOGICALAND && Analysis::HasSideEffects(right)) {
    ...
}
```
