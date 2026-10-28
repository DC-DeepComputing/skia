---
name: skia/modify-by-value-parameters-directly
description: When modifying, normalizing, or reassigning function parameters passed by value in C++ or SkSL, mutate or reassign the parameter directly instead of creating intermediate local copies
rubric: readability
severity: nit
---

**Rule.** Do not create an auxiliary local copy of a function parameter that is
already passed by value (such as a value struct, scalar, raw pointer, or SkSL
vector/color), nor rename the parameter solely to declare a reassignable local
variable. Modify, reassign, or return the by-value parameter directly in place
when sanitizing fields, branching, or updating its value.

**Why.** Parameters passed by value in C++ and shading languages (like
SkSL/GLSL) are already caller-isolated local variables owned by the function
scope. Declaring intermediate local copies (such as usedParam = param,
resultantColor = color, or jprop = defaultProp) introduces redundant
declarations, extra assignments, and renaming boilerplate without improving
safety.

## Examples

Bad:

```cpp
// Redundant local copies when modifying struct fields or vectors
bool Context::submit(SubmitInfo submitInfo) {
    SubmitInfo usedSubmitInfo = submitInfo;
    if (usedSubmitInfo.fSync == SyncToCpu::kYes && !fSharedContext->caps()->allowCpuSync()) {
        usedSubmitInfo.fSync = SyncToCpu::kNo;
    }
    return fQueueManager->submitToGpu(usedSubmitInfo);
}

$pure half4 apply_swizzle(int swizzleType, half4 color) {
    half4 resultantColor = color;
    if (swizzleType == $kReadSwizzleRGB1) {
        resultantColor = color.rgb1;
    }
    return resultantColor;
}

// Renaming parameter to introduce a reassignable local variable
bool AnimatablePropertyContainer::bindImpl(const skjson::ObjectValue* defaultProp) {
    const skjson::ObjectValue* jprop = defaultProp;
    if (const skjson::StringValue* slotID = (*jprop)["sid"]) {
        jprop = (*slot)["p"];
    }
    return jprop != nullptr;
}
```

Good:

```cpp
// Modify struct parameter or vector directly in place
bool Context::submit(SubmitInfo submitInfo) {
    if (submitInfo.fSync == SyncToCpu::kYes && !fSharedContext->caps()->allowCpuSync()) {
        submitInfo.fSync = SyncToCpu::kNo;
    }
    return fQueueManager->submitToGpu(submitInfo);
}

$pure half4 apply_swizzle(int swizzleType, half4 color) {
    if (swizzleType == $kReadSwizzleRGB1) {
        color = color.rgb1;
    }
    return color;
}

// Reassign by-value parameter directly
bool AnimatablePropertyContainer::bindImpl(const skjson::ObjectValue* jprop) {
    if (const skjson::StringValue* slotID = (*jprop)["sid"]) {
        jprop = (*slot)["p"];
    }
    return jprop != nullptr;
}
```
