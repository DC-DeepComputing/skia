---
name: skia/sksl/sksl-evaluate-lvalues-once-in-codegen
description: When generating backend code or lowering SkSL postfix or compound assignment expressions, evaluate lvalue operands only once to avoid duplicating subexpression side effects
rubric: correctness
severity: should_fix
---

**Rule.** In SkSL backend code generators and IR lowering passes, evaluate or
resolve lvalue operands for postfix increment/decrement and compound assignment
operations only once. Do not re-emit or re-evaluate the operand expression to
read and then update it; instead, resolve the assignable lvalue upfront (e.g.,
via makeLValue()), load or push the initial value, duplicate the loaded value if
needed, and write the updated result back through the resolved lvalue.

**Why.** If an lvalue expression contains subexpressions with side effects (such
as array[++idx]++ or foo()[0] += 1), emitting or evaluating the operand
expression multiple times executes those side effects repeatedly, violating
language specification semantics and causing corrupted state or out-of-bounds
memory accesses.

## Examples

Bad:

```cpp
// Bad: Evaluating the operand twice executes subexpression side effects twice (e.g., array[++i]++).
if (!this->pushExpression(*p.operand())) {
    return unsupported();
}
if (!this->pushPrefixExpression(p.getOperator(), *p.operand())) {
    return unsupported();
}
this->discardExpression(p.type().slotCount());
return true;
```

Good:

```cpp
// Good: Resolve the lvalue once, push its value, clone it on the stack to mutate, and store back.
std::unique_ptr<LValue> lvalue = this->makeLValue(*p.operand());
if (!lvalue) {
    return unsupported();
}
lvalue->push(this);
fBuilder.push_clone(p.type().slotCount());
// ... perform increment/decrement on scratch copy ...
lvalue->store(this);
this->discardExpression(p.type().slotCount());
return true;
```
