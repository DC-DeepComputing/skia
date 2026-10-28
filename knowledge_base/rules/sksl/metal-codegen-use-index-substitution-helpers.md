---
name: skia/sksl/metal-codegen-use-index-substitution-helpers
description: In SkSL Metal code generation, use writeWithIndexSubstitution and writeIndexInnerExpression when emitting reused subexpressions instead of manually allocating temporary variables
rubric: readability
severity: should_fix
---

**Rule.** In SkSL Metal code generation, use writeWithIndexSubstitution and
writeIndexInnerExpression when emitting subexpressions that are evaluated or
swizzled multiple times. Rely on these helpers to manage scratch variable
allocation, deduplication, triviality checks, and sequence-expression wrapping
rather than manually creating temporary variables and sequence expressions.

**Why.** MetalCodeGenerator provides writeWithIndexSubstitution and
writeIndexInnerExpression to automatically handle temporary allocation,
deduplication, trivial-expression checks, and sequence-expression formatting for
subexpressions. Open-coding manual temporary variable management and sequence
expressions duplicates complex boilerplate and risks improper expression
wrapping or unnecessary temporary variables.

## Examples

Bad:

```cpp
if (arg.type().isVector() && !Analysis::IsTrivialExpression(arg)) {
    argTemporaries[i] = this->getTempVariable(arg.type());
    needsSequenceExpr = true;
}
if (needsSequenceExpr) {
    this->write("((");
    ...
}
```

Good:

```cpp
this->writeWithIndexSubstitution([&]() {
    ...
    if (arguments[j]->type().isScalar()) {
        this->writeExpression(*arguments[j], Precedence::kSequence);
    } else {
        this->writeIndexInnerExpression(*arguments[j]);
        this->write(kSwizzleChars[i]);
    }
});
```
