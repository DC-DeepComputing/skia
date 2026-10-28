---
name: skia/prefer-skisize-overload-for-mipmap-dimensions
description: When calculating mipmap levels or sizes from an existing SkISize or texture proxy, pass SkISize directly to SkMipmap overloads instead of unpacking width() and height()
rubric: readability
severity: nit
---

**Rule.** Prefer using the SkISize overload of SkMipmap::ComputeLevelCount and
SkMipmap::ComputeLevelSize when 2D dimensions are already available as an
SkISize or from a dimensions() getter. Pass the SkISize object directly rather
than unpacking .width() and .height() as separate arguments.

**Why.** SkMipmap provides overloads taking SkISize directly. Passing an
existing SkISize or .dimensions() call site avoids redundant boilerplate
unpacking of width and height, reducing visual noise and line wrapping while
maintaining type safety.

## Examples

Bad:

```cpp
// Redundant unpacking of SkISize or dimensions()
const SkISize dimensions = textureProxy.dimensions();
int numExpectedLevels =
        SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
SkISize levelSize = SkMipmap::ComputeLevelSize(dimensions.width(), dimensions.height(), level);
```

Good:

```cpp
// Pass SkISize directly to the overload
int numExpectedLevels = SkMipmap::ComputeLevelCount(textureProxy.dimensions()) + 1;
SkISize levelSize = SkMipmap::ComputeLevelSize(textureProxy.dimensions(), level);
```
