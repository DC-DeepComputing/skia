---
name: skia/continue-loop-from-current-index-to-pad-array
description: When padding remaining slots of a fixed-size array after sequential initialization, continue iterating from current index instead of re-iterating with null checks
rubric: readability
severity: nit
---

**Rule.** When padding or filling the remaining slots of a fixed-size array
after sequentially populating entries up to an index, continue iterating from
the current index (`for (; index < kTotal; ++index)`) rather than re-iterating
over the entire array from index 0 with emptiness or null checks. Rely on
sequential initialization order to guarantee that preceding slots are already
valid.

**Why.** Sequentially populating elements guarantees that slots from index 0 up
to the current index are already initialized. Re-iterating from 0 with
conditional null or emptiness checks adds redundant branches and obscures the
sequential initialization invariant.

## Examples

Bad:

```cpp
for (int plane = 0; plane < yuvaProxies.numPlanes(); ++plane) {
    for (int i = 0; i < yuvaInfo.numChannelsInPlane(plane); ++i) {
        imgData.fTextureProxies[textureIndex++] = view.refProxy();
    }
}
for (int tex = 0; tex < 4; ++tex) {
    if (!imgData.fTextureProxies[tex]) {
        imgData.fTextureProxies[tex] = imgData.fTextureProxies[0];
    }
}
```

Good:

```cpp
for (int plane = 0; plane < yuvaProxies.numPlanes(); ++plane) {
    for (int i = 0; i < yuvaInfo.numChannelsInPlane(plane); ++i) {
        imgData.fTextureProxies[textureIndex++] = view.refProxy();
    }
}
for (; textureIndex < 4; ++textureIndex) {
    imgData.fTextureProxies[textureIndex] = imgData.fTextureProxies[0];
}
```
