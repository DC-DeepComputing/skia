---
name: skia/modules/skottie-encapsulate-slot-dispatch
description: In Skottie builders, encapsulate slot ID extraction and value tracking inside shared property dispatchers rather than duplicating JSON slot lookup across attach methods
rubric: design
severity: should_fix
---

**Rule.** In Skottie builders, push slot ID extraction and value tracking down
into shared property dispatchers (e.g., dispatchColorProperty or
dispatchTextProperty) or property binding helpers. Do not duplicate JSON slot
attribute inspection (such as checking for "sid") or track*Value calls across
individual layer and shape attach methods.

**Why.** Individual attach methods should focus on scene hierarchy construction
without repeating JSON attribute parsing. Centralizing slot ID inspection and
slot manager registration in shared property dispatchers ensures consistent slot
binding behavior across builders and eliminates repetitive boilerplate.

## Examples

Bad:

```cpp
// Bad: attach method duplicates slot ID inspection and manual tracking
abuilder->dispatchColorProperty(color_node);
if (const skjson::ObjectValue* colorProp = jpaint["c"]) {
    if (const skjson::StringValue* slotID = (*colorProp)["sid"]) {
        abuilder->trackColorValue(SkString(slotID->begin()), &(color_node->fColor), color_node);
    }
}
```

Good:

```cpp
// Good: shared property dispatcher encapsulates slot lookup and tracking
abuilder->dispatchColorProperty(color_node, jpaint["c"]);
```
