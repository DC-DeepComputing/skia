---
name: skia/general-prefer-dedicated-case-fallthrough-over-default-check
description: When switching on enums or type discriminators, handle specific variants in dedicated case labels with [[fallthrough]] instead of conditional checks inside default
rubric: readability
severity: nit
---

**Rule.** When switching over an enum, AST node discriminator, or type tag
(e.g., `switch (expr.kind())`), handle specific variants in dedicated `case`
labels rather than embedding conditional type or kind checks inside the
`default:` branch. If conditional handling for that variant needs to fall back
to generic handling, use `[[fallthrough]];` explicitly to transfer control to
subsequent cases or `default:`.

**Why.** Switch statements are structured to partition execution by variant.
Nesting type or kind checks inside `default:` hides variant-specific handling in
generic fallback code and obscures control flow; explicit `case` labels with
`[[fallthrough]];` clarify which variants are recognized and make intentional
fallbacks obvious.

## Examples

Bad:

```cpp
switch (expr.kind()) {
    case Expression::Kind::kFieldAccess:
        return chain;
    default: {
        if (expr.is<VariableReference>()) {
            if (const Expression** specialized = findSpecialized(expr)) {
                return this->getAccessChain(**specialized, out);
            }
        }
        return TArray<SpvId>{this->getLValue(expr, out)->getPointer()};
    }
}
```

Good:

```cpp
switch (expr.kind()) {
    case Expression::Kind::kFieldAccess:
        return chain;
    case Expression::Kind::kVariableReference: {
        if (const Expression** specialized = findSpecialized(expr)) {
            return this->getAccessChain(**specialized, out);
        }
        [[fallthrough]];
    }
    default:
        return TArray<SpvId>{this->getLValue(expr, out)->getPointer()};
}
```
