---
name: skia/prefer-sk-make-sp-over-raw-new
description: When instantiating ref-counted objects managed by sk_sp, use sk_make_sp<T>(...) rather than passing bare new to the sk_sp constructor
rubric: readability
severity: nit
---

**Rule.** When instantiating ref-counted objects managed by sk_sp, use
sk_make_sp<T>(...) instead of passing bare new to the sk_sp<T> constructor. Pair
sk_make_sp with auto for local declarations to avoid repeating the type name.

**Why.** sk_make_sp<T>(...) is Skia's canonical factory helper for instantiating
ref-counted objects into an sk_sp, encapsulating raw memory allocation and
avoiding bare calls to new. Pairing sk_make_sp with auto eliminates redundant
type repetition.

## Examples

Bad:

```cpp
sk_sp<SkPngCompositeChunkReader> compositeReader(new SkPngCompositeChunkReader(chunkReader));
```

Good:

```cpp
auto compositeReader = sk_make_sp<SkPngCompositeChunkReader>(chunkReader);
```
