---
name: skia/sksl/include-variable-names-in-sksl-diagnostics
description: When reporting SkSL compiler diagnostics on variable modifications or expressions, include the target identifier and handle anonymous interface blocks via FieldAccess description
rubric: readability
severity: nit
---

**Rule.** When reporting SkSL compiler errors on variable assignments or
expressions, include the target variable or field name in the diagnostic
message. When traversing expressions that may originate from anonymous interface
blocks where the underlying variable name is empty, inspect and format the
enclosing FieldAccess expression instead.

**Why.** Generic compiler diagnostics without the offending identifier leave
developers guessing which variable violated language rules. Furthermore,
variables within anonymous interface blocks have empty names, so falling back to
the FieldAccess description prevents outputting empty quotes in diagnostics.

## Examples

Bad:

```cpp
fErrors->error(expr.fPosition, "cannot assign to pipeline input variable");
```

Good:

```cpp
auto fieldName = [&] {
    return fieldAccess ? fieldAccess->description(OperatorPrecedence::kTopLevel)
                       : std::string(var->name());
};
fErrors->error(expr.fPosition, "cannot modify pipeline input variable '" + fieldName() + "'");
```
