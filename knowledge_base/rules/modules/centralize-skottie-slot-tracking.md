---
name: skia/modules/centralize-skottie-slot-tracking
description: When binding animatable properties or slots in Skottie builders and adapters, delegate slot-ID tracking to dispatch helpers or AnimatablePropertyContainer::bind instead of open-coding slot extraction
rubric: design
severity: should_fix
---

**Rule.** When binding properties or handling slots in Skottie builders and
property adapters, push slot-ID extraction ("sid") and value registration down
into centralized property dispatchers (such as dispatchColorProperty or
dispatchTextProperty) or AnimatablePropertyContainer::bind<T>. Do not manually
inspect slot IDs or invoke SlotManager::track*Value() at individual shape,
paint, or effect call sites.

**Why.** Extracting slot IDs and registering them with SlotManager manually at
each call site duplicates boilerplate and risks inconsistent slot resolution
across properties. Centralizing slot extraction inside dispatch helpers and
container binding guarantees that slot overrides and invalidations are handled
uniformly and keeps builder logic focused on scene graph construction.

## Examples

Bad:

```cpp
sk_sp<sksg::PaintNode> ShapeBuilder::AttachColorStroke(const skjson::ObjectValue& jpaint,
                                                       const AnimationBuilder* abuilder) {
    auto color_node  = sksg::Color::Make(SK_ColorBLACK);
    auto color_paint = AttachStroke(jpaint, abuilder, color_node);
    abuilder->dispatchColorProperty(color_node);
    if (const skjson::ObjectValue* colorProp = jpaint["c"]) {
        if (const skjson::StringValue* slotID = (*colorProp)["sid"]) {
            abuilder->trackColorValue(SkString(slotID->begin()), &(color_node->fColor), color_node);
        }
    }
    return color_paint;
}
```

Good:

```cpp
sk_sp<sksg::PaintNode> ShapeBuilder::AttachColorStroke(const skjson::ObjectValue& jpaint,
                                                       const AnimationBuilder* abuilder) {
    auto color_node  = sksg::Color::Make(SK_ColorBLACK);
    auto color_paint = AttachStroke(jpaint, abuilder, color_node);
    abuilder->dispatchColorProperty(color_node);
    return color_paint;
}
```
