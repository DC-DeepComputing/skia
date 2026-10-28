---
name: skia/sksl/encapsulate-shared-ir-synthesis-in-irhelpers
description: When synthesizing SkSL IR expressions shared across backend code generators, encapsulate AST construction in SkSLIRHelpers rather than duplicating node creation
rubric: design
severity: should_fix
---

**Rule.** When synthesizing SkSL IR expression trees shared across multiple
backend code generators (such as GLSL, Metal, WGSL, or SPIR-V), encapsulate the
IR tree construction in a shared helper in SkSLIRHelpers rather than duplicating
node creation in each generator. Keep code generators focused on emission by
delegating symbol lookup, FieldAccess, and IndexExpression assembly to central
IR helpers.

**Why.** Multiple backends often require identical AST expression structures
when lowering synthetic variables, buffers, or intrinsics into target-specific
constructs. Duplicating manual symbol lookup and IR node synthesis across
backend generators bloats code, introduces subtle discrepancies, and makes AST
layout updates fragile.

## Examples

Bad:

```cpp
case k_loadFloatBuffer_IntrinsicKind: {
    const Symbol* floatBufferArraySymbol =
            this->fProgram.fSymbols->find(this->fCaps.fFloatBufferArrayName);
    SkASSERT(floatBufferArraySymbol && floatBufferArraySymbol->is<FieldSymbol>());
    const FieldSymbol& floatBufferArrayField = floatBufferArraySymbol->as<FieldSymbol>();
    auto floatBufferArrayAccess = FieldAccess::Make(
            fContext, c.position(),
            VariableReference::Make(c.position(), &floatBufferArrayField.owner()),
            floatBufferArrayField.fieldIndex());
    auto indexExpr = IndexExpression::Make(
            fContext, c.position(), std::move(floatBufferArrayAccess), c.arguments()[0]->clone());
    this->writeIndexExpression(*indexExpr);
    return;
}
```

Good:

```cpp
case k_loadFloatBuffer_IntrinsicKind: {
    auto indexExpr = IRHelpers::LoadFloatBuffer(
            fContext, fCaps, c.position(), c.arguments()[0]->clone());
    this->writeExpression(*indexExpr, Precedence::kExpression);
    return;
}
```
