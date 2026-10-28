---
name: skia/modules/take-address-of-skjson-facade-reference
description: When obtaining a pointer from a concrete skjson facade reference, use the address-of operator (&) instead of relying on implicit conversion
rubric: readability
severity: nit
---

**Rule.** When obtaining or passing a pointer from an existing reference to a
concrete skjson facade type (such as const skjson::ObjectValue& or const
skjson::ArrayValue&), explicitly use the address-of operator (&ref). Do not
assign or pass the reference directly relying on skjson::Value's implicit
conversion operator (operator const T*()).

**Why.** skjson::Value defines a templated user-defined conversion operator
(operator const T*()) designed for dynamic querying and downcasting from untyped
Value instances, which performs a runtime type check (this->is<T>()). When code
already holds a reference to a statically known concrete facade type, relying on
implicit conversion incurs redundant runtime type-checking overhead and obscures
taking the address of an existing object.

## Examples

Bad:

```cpp
void AnimationBuilder::loadFootageAsset(const skjson::ObjectValue& defaultJImage) const {
    // Relies on inherited implicit conversion operator const ObjectValue*()
    const skjson::ObjectValue* jimage = defaultJImage;
    processImage(jimage);
}
```

Good:

```cpp
void AnimationBuilder::loadFootageAsset(const skjson::ObjectValue& defaultJImage) const {
    // Explicitly takes the address of the known concrete reference
    const skjson::ObjectValue* jimage = &defaultJImage;
    processImage(jimage);
}
```
