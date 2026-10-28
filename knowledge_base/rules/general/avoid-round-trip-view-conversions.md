---
name: skia/avoid-round-trip-view-conversions
description: When preparing buffer data across branches for FFI functions expecting a slice type, assign directly to the target slice rather than round-tripping through SkSpan
rubric: readability
severity: nit
---

**Rule.** When staging or transforming buffer data across conditional branches
to pass to a function expecting a specific slice view type (such as
rust::Slice), declare and assign directly to a local variable of the target
slice type in each branch. Do not reassign an intermediate SkSpan across
branches only to convert it back to the target slice type at the call site.

**Why.** Round-tripping between SkSpan and rust::Slice creates redundant view
object constructions and obscures data flow with confusing type ping-pong
between container view abstractions. Assigning directly to a variable of the
destination slice type across branches clarifies intent and eliminates
superfluous conversions.

## Examples

Bad:

```cpp
// row is SkSpan<const uint8_t>
switch (this->fExtraRowTransform) {
    case kNone_ExtraRowTransform:
        break;
    case kRgbaToRgb_ExtraRowTransform:
        // ... transform into fExtraRowBuffer ...
        row = rust::Slice<const uint8_t>(fExtraRowBuffer);
        break;
}
return fStreamWriter->write(rust::Slice<const uint8_t>(row)) == rust_png::EncodingResult::Success;
```

Good:

```cpp
rust::Slice<const uint8_t> rustRow;
switch (this->fExtraRowTransform) {
    case kNone_ExtraRowTransform:
        rustRow = rust::Slice<const uint8_t>(row);
        break;
    case kRgbaToRgb_ExtraRowTransform:
        // ... transform into fExtraRowBuffer ...
        rustRow = rust::Slice<const uint8_t>(fExtraRowBuffer);
        break;
}
return fStreamWriter->write(rustRow) == rust_png::EncodingResult::Success;
```
