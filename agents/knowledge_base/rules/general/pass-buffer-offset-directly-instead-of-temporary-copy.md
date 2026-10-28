---
name: skia/pass-buffer-offset-directly-instead-of-temporary-copy
description: When passing contiguous sub-arrays to functions accepting pointer or array parameters, pass the buffer offset or sub-span directly instead of copying into a temporary buffer
rubric: safety
severity: should_fix
---

**Rule.** When passing contiguous sub-arrays or slices of existing buffers to
functions taking pointer or array parameters, pass the buffer pointer offset
(such as ptr + index) or sub-span directly. Do not declare temporary local stack
arrays and copy elements into them before calling the function.

**Why.** In C++, array function parameters decay to pointers. When contiguous
elements already reside in memory, copying them into a local stack buffer
introduces unnecessary memory copying and stack allocation overhead. Passing the
pointer offset or sub-span directly avoids redundant copying while allowing the
callee to access the sub-array in place.

## Examples

Bad:

```cpp
// Bad: Copying contiguous elements into a temporary local array
SkPoint clip[4];
if (imageSet[i].fHasClip) {
    for (int j = 0; j < 4; ++j) {
        clip[j] = dstClips[dstClipIndex + j];
    }
}
this->onDrawEdgeAAQuad(drawDst, clip, ...);
```

Good:

```cpp
// Good: Pass buffer pointer offset directly without copying
const SkPoint* clip = imageSet[i].fHasClip ? dstClips + dstClipIndex : nullptr;
this->topDevice()->drawEdgeAAImageSet(&imageSet[i], 1, clip, ...);
```
