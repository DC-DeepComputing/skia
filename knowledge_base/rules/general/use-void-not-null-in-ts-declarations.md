---
name: skia/use-void-not-null-in-ts-declarations
description: When declaring void-returning functions or methods in TypeScript declaration files (*.d.ts), specify void rather than null or undefined as the return type
rubric: correctness
severity: nit
---

**Rule.** In TypeScript declaration files (*.d.ts), use `void` rather than
`null` (or `undefined`) as the return type for functions and methods that do not
return a value. Do not specify `null` for operations whose underlying
implementation returns nothing.

**Why.** In JavaScript and Embind WebAssembly bindings, functions with no return
value evaluate to `undefined` at runtime. Typing a non-returning function as
`null` in TypeScript declarations misrepresents runtime return semantics,
misleads API consumers, and causes type-checking errors.

## Examples

Bad:

```cpp
interface ManagedAnimation {
    enableEditor(enable: boolean): null;
}
```

Good:

```cpp
interface ManagedAnimation {
    enableEditor(enable: boolean): void;
}
```
