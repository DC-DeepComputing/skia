---
name: skia/api/prefer-reinterpret-cast-over-c-style-cast
description: When converting numeric values, enums, or pointers, do not use C-style casts; use static_cast, reinterpret_cast, or Skia helpers like SkToInt
rubric: readability
severity: should_fix
---

**Rule.** Do not use C-style casts `(Type)val`. For pointer conversions, use
explicit C++ casts such as `static_cast<T*>` for downcasts or
`reinterpret_cast<T*>` for reinterpreting byte buffers instead of `(T*)` to
maintain type safety and avoid silently casting away `const`. For numeric,
narrowing, or enum conversions (such as integer to enum, `size_t` container
sizes to `int`, or narrowing to `unsigned short`), use `static_cast<T>`, use
Skia's checked conversion helpers (such as `SkToInt`, `SkToSizeT`, or
`SkTo<T>`), or align variable and parameter types to eliminate the cast
entirely.

**Why.** C-style casts bypass C++ type safety, obscure conversion intent, and
violate coding conventions. When applied to pointers, they can silently combine
static_cast, reinterpret_cast, and const_cast, discarding const qualifiers or
masking invalid conversions. When applied to numbers or enums, C-style casts
allow arbitrary unsafe conversions without compiler validation and bypass Skia's
debug bounds-checking assertions in SkToInt / SkTo<T> / SkToSizeT.

## Examples

Bad:

```cpp
// Bad: C-style casts bypass const-safety or mask numeric truncation and enum mismatches.
const uint8_t* bytes() const {
    return (uint8_t*)this->data();
}

template <typename T>
T* cast() {
    return (T*)fSpan.data();
}

SkSLTraceUtils::WriteTrace((SkSL::DebugTracePriv*)debugTrace.get(), &traceFile);

origin = (SkEncodedOrigin)propValue.uiVal;

return MakeRadial(..., (int)colors.size(), ...);

CreateFontPackage(..., (unsigned short)keepList.size(), ...);
```

Good:

```cpp
// Good: Explicit C++ casts preserve constness; Skia helpers assert bounds.
const uint8_t* bytes() const {
    return reinterpret_cast<const uint8_t*>(this->data());
}

template <typename T>
T* cast() {
    return reinterpret_cast<T*>(fSpan.data());
}

SkSLTraceUtils::WriteTrace(static_cast<SkSL::DebugTracePriv*>(debugTrace.get()), &traceFile);

origin = static_cast<SkEncodedOrigin>(propValue.uiVal);

// Use checked conversion helpers or align types to avoid casting:
return MakeRadial(..., SkToInt(colors.size()), ...);

CreateFontPackage(..., SkTo<unsigned short>(keepList.size()), ...);
```
