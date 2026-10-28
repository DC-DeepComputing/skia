---
name: skia/api/prefer-dynamic-memory-wstream
description: When buffering or accumulating stream data in memory, use or extend SkDynamicMemoryWStream instead of introducing custom in-memory SkWStream subclasses
rubric: design
severity: should_fix
---

**Rule.** When accumulating or buffering stream output in memory, use Skia's
canonical in-memory write stream SkDynamicMemoryWStream rather than introducing
custom in-memory SkWStream subclasses (such as std::vector- or
flat-buffer-backed streams). If consumers require a specific container or flat
representation, extend SkDynamicMemoryWStream with conversion helpers (such as
detachAsVector() or detachAsData()) or use its existing reset/copy methods
instead of writing a new stream class.

**Why.** SkDynamicMemoryWStream is Skia's established, canonical in-memory
stream abstraction. Its chunked, block-based allocation avoids repeated memory
reallocations and buffer resizing during incremental streaming, outperforming
naive dynamically growing vector buffers while preventing proliferation of
redundant stream adapter classes across the codebase.

## Examples

Bad:

```cpp
// Bad: Introducing a redundant in-memory SkWStream subclass backed by std::vector
class SK_API SkVectorWStream : public SkWStream {
public:
    bool write(const void* buffer, size_t size) override {
        const uint8_t* bytes = static_cast<const uint8_t*>(buffer);
        fVector.insert(fVector.end(), bytes, bytes + size);
        return true;
    }
    std::vector<uint8_t> detachAsVector() { return std::exchange(fVector, {}); }
private:
    std::vector<uint8_t> fVector;
};
```

Good:

```cpp
// Good: Use SkDynamicMemoryWStream and its canonical extraction methods
SkDynamicMemoryWStream stream;
// ... write to stream ...
std::vector<uint8_t> vec = stream.detachAsVector();
// Or detach as SkData:
sk_sp<SkData> data = stream.detachAsData();
```
