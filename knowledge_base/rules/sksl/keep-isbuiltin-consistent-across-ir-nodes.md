---
name: skia/sksl/keep-isbuiltin-consistent-across-ir-nodes
description: When defining predicates on SkSL IR types or symbols, keep isBuiltin() reserved for module origin and name container checks specifically (e.g. isInRootSymbolTable)
rubric: readability
severity: should_fix
---

**Rule.** In SkSL IR types and symbols, keep isBuiltin() consistent across AST
nodes to denote entities originating outside user program code (such as modules
and built-ins). Do not introduce near-synonymous predicate helpers like
isInBuiltinTypes() that create ambiguity with isBuiltin(); name methods after
the concrete container, table, or condition being checked (such as
isInRootSymbolTable()).

**Why.** Across SkSL IR nodes (including Type, Variable, FunctionDeclaration,
and SymbolTable), isBuiltin() carries an established meaning: whether an entity
originates from built-in module code. Introducing near-synonymous names like
isInBuiltinTypes() on Type creates confusion between external module origin and
pre-population in the root symbol table, obscuring the AST's invariant
semantics.

## Examples

Bad:

```cpp
// Bad: near-synonymous names on SkSL::Type confuse module origin with symbol table location
virtual bool isBuiltin() const;
bool isInBuiltinTypes() const {
    return !(this->isArray() || this->isStruct());
}
```

Good:

```cpp
// Good: isBuiltin() denotes module origin; root table membership is explicitly named
virtual bool isBuiltin() const;
bool isInRootSymbolTable() const {
    return !(this->isArray() || this->isStruct());
}
```
