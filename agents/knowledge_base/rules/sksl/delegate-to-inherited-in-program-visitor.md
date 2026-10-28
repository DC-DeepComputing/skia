---
name: skia/sksl/delegate-to-inherited-in-program-visitor
description: In SkSL ProgramVisitor or ProgramWriter subclasses, delegate to INHERITED::visit* rather than returning false directly, unless intentionally halting traversal into child nodes
rubric: correctness
severity: should_fix
---

**Rule.** When overriding visitExpression, visitStatement, or
visitProgramElement in SkSL::ProgramVisitor or SkSL::ProgramWriter subclasses,
return INHERITED::visit*(...) to recurse into child nodes. Do not return false
directly after inspecting or mutating a node unless deliberately arresting
recursion into child subexpressions or nested statements.

**Why.** In SkSL's TProgramVisitor and ProgramWriter, base visit* methods
perform the recursive descent into child AST nodes, returning false to continue
traversal and true to abort. Returning false directly from an override bypasses
the base class recursion, unintentionally halting traversal at that node and
failing to visit nested subexpressions or statements.

## Examples

Bad:

```cpp
bool visitExpression(const Expression& expr) override {
    if (expr.is<FunctionCall>()) {
        fCallMap[&expr.as<FunctionCall>()] = fIndex;
        // Bug: returning false directly halts traversal and skips argument subexpressions
        return false;
    }
    return INHERITED::visitExpression(expr);
}
```

Good:

```cpp
bool visitExpression(const Expression& expr) override {
    if (expr.is<FunctionCall>()) {
        fCallMap[&expr.as<FunctionCall>()] = fIndex;
        // Traverses into child argument subexpressions
        return INHERITED::visitExpression(expr);
    }
    return INHERITED::visitExpression(expr);
}
```
