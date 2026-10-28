---
name: skia/ports/validate-font-scanner-instance-index
description: When implementing SkFontScanner::scanInstance, validate that instanceIndex does not exceed the count of named instances, returning false on out-of-bounds queries
rubric: correctness
severity: should_fix
---

**Rule.** When implementing SkFontScanner::scanInstance in font backends,
validate that instanceIndex does not exceed the number of named instances
(num_named_instances()), returning false on out-of-bounds queries before reading
instance data. Treat instanceIndex == 0 as the default instance and 1 through
numInstances as named instances.

**Why.** SkFontScanner defines instanceIndex == 0 as the default instance and
indices 1 to numInstances as named instances. Querying an instanceIndex greater
than the available named instances is an invalid argument that will read invalid
or non-existent coordinates unless rejected early with false.

## Examples

Bad:

```cpp
bool SkFontScanner_Custom::scanInstance(SkStreamAsset* stream, int faceIndex, int instanceIndex, ...) {
    auto num = SkToInt(bridgeFontFace->num_named_instances());
    if (instanceIndex == 0) {
        // Query default instance
    } else {
        // Unchecked access for named instances where instanceIndex may exceed num
        bridgeFontFace->get_instance_coordinates(instanceIndex - 1);
    }
    return true;
}
```

Good:

```cpp
bool SkFontScanner_Custom::scanInstance(SkStreamAsset* stream, int faceIndex, int instanceIndex, ...) {
    auto num = SkToInt(bridgeFontFace->num_named_instances());
    if (instanceIndex > num) {
        return false;
    } else if (instanceIndex == 0) {
        // Query default instance
    } else {
        bridgeFontFace->get_instance_coordinates(instanceIndex - 1);
    }
    return true;
}
```
