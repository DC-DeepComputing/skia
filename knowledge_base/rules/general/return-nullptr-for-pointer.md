---
name: skia/return-nullptr-for-pointer
description: When returning null from a function with a pointer return type, return explicit nullptr rather than temporary sentinel objects or implicit conversions
rubric: readability
severity: nit
---

**Rule.** When a function returns a pointer type (such as const T*), explicitly
return nullptr to signal absence, failure, or a null result. Do not return
temporary sentinel objects (such as skjson::NullValue()), empty braces {}, or
values that rely on user-defined implicit conversion operators to yield a null
pointer.

**Why.** Returning temporary sentinel objects to produce a null pointer relies
on implicit conversion operators, which obscures intent and can mislead readers
into wondering whether a pointer to a temporary is being returned. Returning
nullptr communicates pointer semantics directly, eliminates ambiguity, and
avoids redundant object construction and dynamic type-tag checking overhead.

## Examples

Bad:

```cpp
const skjson::StringValue* AnimatablePropertyContainer::getSlotID(const skjson::ObjectValue* jobj) {
    if (jobj) {
        if (const skjson::StringValue* sid = (*jobj)["sid"]) {
            fHasSlotID = true;
            return sid;
        }
    }
    // Relying on temporary sentinel implicit conversion to null pointer
    return skjson::NullValue();
}
```

Good:

```cpp
const skjson::StringValue* AnimatablePropertyContainer::getSlotID(const skjson::ObjectValue* jobj) {
    if (jobj) {
        if (const skjson::StringValue* sid = (*jobj)["sid"]) {
            fHasSlotID = true;
            return sid;
        }
    }
    return nullptr;
}
```
