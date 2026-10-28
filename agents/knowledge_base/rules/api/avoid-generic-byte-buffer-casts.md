---
name: skia/api/avoid-generic-byte-buffer-casts
description: When designing APIs for opaque byte buffer containers, do not expose generic template pointer or span casts; restrict views to byte spans
rubric: correctness
severity: should_fix
---

**Rule.** Do not expose generic member template methods (such as cast<T>(),
as<T>(), or span<T>()) that cast opaque byte buffer storage directly to
arbitrary pointer or span types (T* or SkSpan<const T>). Restrict buffer
accessors to byte-oriented views (such as bytes() returning const uint8_t* and
byteSpan() returning SkSpan<const uint8_t>), or require callers to safely
deserialize or copy the payload using functions like memcpy or sk_bit_cast.

**Why.** Under C++ strict aliasing rules, accessing memory through pointer types
other than char-like types (char, unsigned char, std::byte, or uint8_t) is
undefined behavior unless the dynamic type of the object matches T. Exposing
generic template casts on raw byte storage also introduces alignment faults on
strict-alignment architectures.

## Examples

Bad:

```cpp
class DataBuffer {
public:
    template <typename T>
    const T* cast() const {
        return reinterpret_cast<const T*>(fSpan.data());
    }
    template <typename T>
    SkSpan<const T> span() const {
        return {reinterpret_cast<const T*>(fSpan.data()), this->size() / sizeof(T)};
    }
private:
    SkSpan<const uint8_t> fSpan;
};
```

Good:

```cpp
class DataBuffer {
public:
    const uint8_t* bytes() const {
        return reinterpret_cast<const uint8_t*>(fSpan.data());
    }
    SkSpan<const uint8_t> byteSpan() const {
        return fSpan;
    }
private:
    SkSpan<const uint8_t> fSpan;
};
```
