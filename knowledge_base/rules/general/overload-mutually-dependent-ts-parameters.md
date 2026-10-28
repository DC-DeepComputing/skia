---
name: skia/overload-mutually-dependent-ts-parameters
description: When declaring mutually dependent parameters in TypeScript definitions, use function overloads or composite types rather than independent optional arguments
rubric: correctness
severity: should_fix
---

**Rule.** In TypeScript type declarations (*.d.ts), do not declare mutually
dependent parameters as independently optional (e.g., (a?: T, b?: U)) when the
underlying implementation requires either all or none of them. Use function
overloads or a composite tuple/object parameter to enforce that interdependent
arguments are supplied together.

**Why.** Declaring interdependent parameters as independently optional allows
callers to supply only a subset of arguments (such as srcRect without dstRect).
This passes compile-time type checks but violates the underlying
implementation's invariants, risking runtime errors, failed WebAssembly
bindings, or silently ignored parameters.

## Examples

Bad:

```cpp
MakeImage(img: Image, sampling: FilterOptions | CubicResampler,
          srcRect?: InputRect, dstRect?: InputRect): ImageFilter | null;
```

Good:

```cpp
MakeImage(img: Image, sampling: FilterOptions | CubicResampler): ImageFilter | null;
MakeImage(img: Image, sampling: FilterOptions | CubicResampler,
          srcRect: InputRect, dstRect: InputRect): ImageFilter | null;
```
