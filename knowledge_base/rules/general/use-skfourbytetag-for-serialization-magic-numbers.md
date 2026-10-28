---
name: skia/use-skfourbytetag-for-serialization-magic-numbers
description: When defining serialization magic numbers or format constants, use constexpr SkFourByteTag instead of char arrays, and keep internal tags private or implementation-local
rubric: design
severity: should_fix
---

**Rule.** When defining magic numbers, file signatures, or chunk header tags for
serialization formats, declare them as constexpr SkFourByteTag constants
initialized with SkSetFourByteTag('a', 'b', 'c', 'd'). Validate and serialize
them using 32-bit stream operations (such as readU32 and write32) rather than
unsized char[] arrays, byte buffers, or memcmp checks. Additionally, do not
expose internal serialization constants, format magic numbers, or version tags
in the public interface of a class; keep them private to the class or local to
the .cpp implementation file.

**Why.** Declaring unsized char arrays introduces incomplete types in headers,
requires manual size tracking, and forces raw byte buffer operations instead of
efficient 32-bit integer comparisons. Furthermore, exposing internal
serialization magic numbers and version tags in public class interfaces leaks
file-format implementation details and unnecessarily expands the library's
public API surface.

## Examples

Bad:

```cpp
// Header
class SkCapture : public SkRefCnt {
public:
    // Bad: Exposes format magic and version in public interface; uses char[]
    static const char kMagic[];
    static const size_t kMagicSize = 8;
    static const uint32_t kVersion = 0;
};

// Source
const char SkCapture::kMagic[] = {'s', 'k', 'i', 'a', 'c', 'a', 'p', 't'};
char magic[sizeof(kMagic)];
if (stream.read(magic, sizeof(kMagic)) != sizeof(kMagic) ||
    memcmp(magic, kMagic, sizeof(kMagic)) != 0) {
    return false;
}
```

Good:

```cpp
// Header
class SkCapture : public SkRefCnt {
private:
    // Good: Internal format version encapsulated in private section
    static const uint32_t kVersion = 0;
};

// Source (TU-local file scope)
static constexpr SkFourByteTag kMagic1 = SkSetFourByteTag('s', 'k', 'i', 'a');
static constexpr SkFourByteTag kMagic2 = SkSetFourByteTag('c', 'a', 'p', 't');

uint32_t magic1 = 0, magic2 = 0;
if (!stream.readU32(&magic1) || !stream.readU32(&magic2) ||
    magic1 != kMagic1 || magic2 != kMagic2) {
    return false;
}
```
