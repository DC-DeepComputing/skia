---
name: skia/avoid-inlining-sk-sp-forward-decl
description: When defining functions in headers taking or returning sk_sp<T> by value where T is forward-declared, implement them in .cpp files instead of inline
rubric: correctness
severity: blocking
---

**Rule.** Do not implement functions inline in header files if they accept,
return, or instantiate sk_sp<T> by value when T is only forward-declared.
Declare the function in the header and define it in a .cpp source file where the
defining header for T is included.

**Why.** Destroying sk_sp<T> calls SkSafeUnref(fPtr) which invokes obj->unref(),
requiring T to be a complete type. Inlining functions that pass or return
sk_sp<T> by value in headers with forward-declared T causes compilation failures
in translation units that include the header without T's definition.

## Examples

Bad:

```cpp
// In SkGainmapShader.h (where SkColorSpace is only forward-declared):
class SkColorSpace;

class SK_API SkGainmapShader {
public:
    static sk_sp<SkShader> Make(const sk_sp<const SkImage>& baseImage,
                                const SkGainmapInfo& gainmapInfo,
                                sk_sp<SkColorSpace> dstColorSpace) {
        return Make(baseImage, gainmapInfo); // Compilation error: dstColorSpace destruction requires complete SkColorSpace
    }
};
```

Good:

```cpp
// In SkGainmapShader.h:
class SkColorSpace;

class SK_API SkGainmapShader {
public:
    static sk_sp<SkShader> Make(const sk_sp<const SkImage>& baseImage,
                                const SkGainmapInfo& gainmapInfo,
                                sk_sp<SkColorSpace> dstColorSpace);
};

// In SkGainmapShader.cpp (where SkColorSpace.h is included):
#include "include/core/SkColorSpace.h"
#include "include/private/SkGainmapShader.h"

sk_sp<SkShader> SkGainmapShader::Make(const sk_sp<const SkImage>& baseImage,
                                      const SkGainmapInfo& gainmapInfo,
                                      sk_sp<SkColorSpace> dstColorSpace) {
    return Make(baseImage, gainmapInfo);
}
```
