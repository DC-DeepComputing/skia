---
name: skia/codec/avoid-recursive-codec-instantiation-on-nested-images
description: When decoding embedded auxiliary streams or gainmaps, query child codec metadata non-recursively via onGetGainmapInfo rather than calling onGetGainmapCodec to prevent recursion
rubric: safety
severity: should_fix
---

**Rule.** When inspecting embedded streams or auxiliary images (such as HDR
gainmaps) from untrusted inputs in image codecs, do not call recursive codec
instantiation methods (such as `onGetGainmapCodec`) on child codecs to check for
metadata. Instead, query metadata through non-recursive methods (such as
`onGetGainmapInfo`) or enforce a strict depth limit to prevent stack exhaustion
from maliciously nested payloads.

**Why.** Untrusted image files may contain maliciously crafted nested payloads,
such as gainmap images recursively embedding further gainmaps. Invoking
recursive codec creation methods on child codecs risks unbounded recursion,
stack overflow, and denial-of-service crashes.

## Examples

Bad:

```cpp
// Vulnerable: calling onGetGainmapCodec on child codec risks unbounded recursion
bool nestedResult = codec->onGetGainmapCodec(info, nullptr);
if (nestedResult && gainmapCodec) {
    *gainmapCodec = std::move(codec);
}
return nestedResult;
```

Good:

```cpp
// Safe: queries child codec metadata without recursively creating child gainmaps
bool hasInfo = codec->onGetGainmapInfo(info);
if (hasInfo && gainmapCodec) {
    *gainmapCodec = std::move(codec);
}
return hasInfo;
```
