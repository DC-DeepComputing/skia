---
name: skia/ports/delegate-onopenstream-to-proxy-typeface
description: When implementing onOpenStream in typeface wrappers or proxies, delegate to the underlying typeface's onOpenStream instead of re-opening the font stream from disk
rubric: safety
severity: should_fix
---

**Rule.** When implementing onOpenStream() in an SkTypeface wrapper, proxy, or
platform adapter backed by an underlying typeface, delegate directly to the
underlying typeface's onOpenStream() instead of re-opening the font file from
disk (such as via SkStream::MakeFromFile()).

**Why.** The underlying typeface already manages the font data and can duplicate
its existing stream (e.g. via SkStream::duplicate()), avoiding redundant disk
I/O, filesystem path lookups, and file descriptor consumption.

## Examples

Bad:

```cpp
std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
    *ttcIndex = get_int(fPattern, FC_INDEX, 0);
    const char* filename = get_string(fPattern, FC_FILE);
    return SkStream::MakeFromFile(filename);
}
```

Good:

```cpp
std::unique_ptr<SkStreamAsset> onOpenStream(int* ttcIndex) const override {
    return fProxy->onOpenStream(ttcIndex);
}
```
