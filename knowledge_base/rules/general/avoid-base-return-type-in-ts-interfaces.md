---
name: skia/avoid-base-return-type-in-ts-interfaces
description: When declaring methods on extensible TypeScript base interfaces, do not return the base type; use polymorphic this, F-bounded generics, or omit the method
rubric: correctness
severity: should_fix
---

**Rule.** In TypeScript type definitions, do not declare methods on a base
interface that return the base type itself (such as clone(): Base<T>) when
derived interfaces extend it, as invoking them on derived instances strips
subtype members and causes type-checking errors. Either use polymorphic this
(clone(): this), parameterize the base interface with the derived type via
F-bounded polymorphism, or omit the method if it is not part of the public
JavaScript API.

**Why.** TypeScript does not automatically specialize return types of inherited
methods. If a base interface defines a method returning the base type, calling
that method on an instance of a derived interface downcasts the resulting type
to the base interface, causing TypeScript compilation errors when subsequent
code accesses properties or methods exclusive to the derived interface.

## Examples

Bad:

```cpp
export interface EmbindObject<T extends string> {
    _type: T;
    // Calling clone() on derived instances (e.g. Path) returns EmbindObject<T>, stripping Path members.
    clone(): EmbindObject<T>;
    delete(): void;
}

export interface Path extends EmbindObject<"Path"> {
    lineTo(x: number, y: number): void;
}
```

Good:

```cpp
// Option 1: Return polymorphic 'this' so derived instances preserve their specialized type.
export interface EmbindObject<T extends string> {
    _type: T;
    clone(): this;
    delete(): void;
}

// Option 2: Omit methods that are internal or unsupported in the public JS API.
export interface EmbindObject<T extends string> {
    _type: T;
    delete(): void;
}
```
