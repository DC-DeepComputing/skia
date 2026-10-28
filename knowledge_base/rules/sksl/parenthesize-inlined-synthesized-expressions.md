---
name: skia/sksl/parenthesize-inlined-synthesized-expressions
description: When synthesizing inlined polyfills or alternative operators in shader code generators, enclose multi-term expressions in parentheses and emit subexpressions using the synthesized operator's precedence
rubric: correctness
severity: should_fix
---

**Rule.** When synthesizing inlined polyfills, multi-term expressions, or
alternative operators in shader backend code generators, ensure expressions are
correctly parenthesized to preserve operator precedence. Wrap synthesized
multi-term expressions in outer parentheses so caller operators cannot bind to
internal terms. Additionally, when lowering an expression using an alternative
operator (such as rewriting unary negation into scalar multiplication by -1.0),
pass the synthesized operator's precedence (e.g., Precedence::kMultiplicative)
rather than the original AST operator's precedence to recursive expression
writers.

**Why.** Synthesized multi-term expressions lack atomic variable isolation and
can bind unexpectedly to caller operators if left unparenthesized. Similarly,
passing an original AST operator's precedence instead of the synthesized
operator's precedence to recursive expression formatters causes subexpressions
to omit required parentheses or parenthesize incorrectly.

## Examples

Bad:

```cpp
// Bad: Synthesized polyfill unparenthesized; rewritten operator passes original AST precedence
case k_mod_IntrinsicKind:
    return arg0 + " - " + arg1 + " * floor(" + arg0 + " / " + arg1 + ")";

if (needsNegation) {
    this->write("-1.0 * ");
} else {
    this->write(p.getOperator().tightOperatorName());
}
this->writeExpression(*p.operand(), Precedence::kPrefix);
```

Good:

```cpp
// Good: Synthesized expressions wrapped in parentheses and recursive calls use synthesized precedence
case k_mod_IntrinsicKind:
    return "(" + arg0 + " - " + arg1 + " * floor(" + arg0 + " / " + arg1 + "))";

if (needsNegation) {
    this->write("-1.0 * ");
    this->writeExpression(*p.operand(), Precedence::kMultiplicative);
} else {
    this->write(p.getOperator().tightOperatorName());
    this->writeExpression(*p.operand(), Precedence::kPrefix);
}
```
