---
name: skia/ports/null-check-optional-font-scanner-out-parameters
description: When implementing SkFontScanner methods like scanFile, scanFace, or scanInstance, guard optional out-pointers with null checks before writing to them
rubric: correctness
severity: blocking
---

**Rule.** When implementing SkFontScanner interface methods (such as scanFile,
scanFace, and scanInstance), treat output pointer parameters as optional by
guarding writes with null checks before dereferencing them, and return a boolean
indicating stream or face scan validity.

**Why.** Callers such as font managers and format probes frequently pass nullptr
for output parameters when only validating stream format or face viability.
Writing to output pointers unconditionally causes null pointer dereference
crashes when callers omit optional arguments.

## Examples

Bad:

```cpp
bool SkFontScanner_Custom::scanFace(SkStreamAsset* stream,
                                     int faceIndex,
                                     int* numInstances) const {
    // Unchecked dereference assumes caller always requests instance count
    *numInstances = font_backend::get_num_instances(stream, faceIndex);
    return true;
}
```

Good:

```cpp
bool SkFontScanner_Custom::scanFace(SkStreamAsset* stream,
                                     int faceIndex,
                                     int* numInstances) const {
    if (numInstances) {
        *numInstances = font_backend::get_num_instances(stream, faceIndex);
    }
    return true;
}
```
