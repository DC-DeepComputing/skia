---
name: skia/avoid-arena-allocations-before-fallible-operations
description: When preparing parameters for operations that may fail or fall back, avoid allocating temporary objects in SkArenaAlloc; pass by value or stack instead
rubric: safety
severity: should_fix
---

**Rule.** Do not allocate temporary parameter objects from an arena (such as
SkArenaAlloc) before operations that may fail or fall back. Pass parameter
objects on the stack or by value, and design target objects to store them by
value rather than allocating standalone arena objects that cannot be reclaimed
on failure.

**Why.** SkArenaAlloc does not support individual deallocations; allocated
memory cannot be reclaimed until the arena is reset or destroyed. Eagerly
allocating parameter objects in an arena prior to a fallible constructor or
factory call wastes arena capacity on fallback paths.

## Examples

Bad:

```cpp
if (paint->getShader()) {
    // Bad: allocating parameter in arena before fallible context creation wastes arena memory on fallback
    auto mRec = alloc->make<SkShaderBase::MatrixRec>(ctm);
    shaderContext = as_SB(paint->getShader())->makeContext(
            {*paint, *mRec, device.colorType(), device.colorSpace(), props},
            alloc);
    if (!shaderContext) {
        return create_SkRP_or_SkVMBlitter();
    }
}
```

Good:

```cpp
if (paint->getShader()) {
    // Good: pass parameter by value on the stack so fallback does not waste arena memory
    shaderContext = as_SB(paint->getShader())->makeContext(
            {paint->getAlpha(),
             SkShaders::MatrixRec(ctm),
             device.colorType(),
             device.colorSpace(),
             props},
            alloc);
    if (!shaderContext) {
        return CreateSkRPBlitter();
    }
}
```
