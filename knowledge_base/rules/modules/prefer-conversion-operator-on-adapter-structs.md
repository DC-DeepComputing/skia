---
name: skia/modules/prefer-conversion-operator-on-adapter-structs
description: When defining dedicated adapter or binding structs that convert into engine types, prefer a member conversion operator over standalone conversion helper functions
rubric: readability
severity: nit
---

**Rule.** For dedicated adapter or binding structs whose primary purpose is to
convert into an engine type, prefer a member conversion operator (`operator
TargetType() const`) over a standalone conversion helper function
(`toTargetType(adapter)`). This enables adapter instances to be passed directly
to engine APIs accepting the target type without intermediate conversion
boilerplate.

**Why.** Implementing a conversion operator directly on a dedicated adapter
struct encapsulates the translation logic, eliminating standalone helper
functions and redundant intermediate local variables at call sites.

## Examples

Bad:

```cpp
struct SimpleSlottableTextProperty {
    ...
};

skottie::TextPropertyValue toTextProperty(SimpleSlottableTextProperty simple) {
    skottie::TextPropertyValue textProperty;
    ...
    return textProperty;
}

bool setTextSlot(const std::string& slotID, SimpleSlottableTextProperty t) {
    auto textProp = toTextProperty(t);
    return fSlotMgr->setTextSlot(SkString(slotID), textProp);
}
```

Good:

```cpp
struct SimpleSlottableTextProperty {
    ...
    operator skottie::TextPropertyValue() const {
        skottie::TextPropertyValue textProperty;
        ...
        return textProperty;
    }
};

bool setTextSlot(const std::string& slotID, SimpleSlottableTextProperty t) {
    return fSlotMgr->setTextSlot(SkString(slotID), t);
}
```
