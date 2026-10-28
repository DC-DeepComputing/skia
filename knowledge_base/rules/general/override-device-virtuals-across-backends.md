---
name: skia/override-device-virtuals-across-backends
description: When adding or calling new virtual capabilities on SkDevice or SkCanvas, implement fallback overrides across all backends including PDF and XPS to prevent test failures
rubric: correctness
severity: blocking
---

**Rule.** When adding new virtual methods or capabilities to SkDevice (such as
baseRecorder()) that SkCanvas callers or GMs rely upon, implement or override
them across all SkDevice subclasses—including vector and document backends like
SkPDFDevice, SkXPSDevice, and SkSVGDevice. Provide an appropriate fallback (such
as returning skcpu::Recorder::TODO()) rather than returning null when callers or
GM tests require a non-null object.

**Why.** GMs and tests run across all supported device backends on continuous
integration bots. If a new virtual method on SkDevice defaults to nullptr and is
only overridden in raster and GPU backends, vector and document devices like PDF
will return null and trigger assertion failures or crashes during test runs.

## Examples

Bad:

```cpp
// src/core/SkDevice.h:
virtual SkRecorder* baseRecorder() const { return nullptr; }
// Overridden in SkBitmapDevice and GPU devices, but omitted in SkPDFDevice.

// gm/makecolorspace.cpp:
auto recorder = canvas->baseRecorder();
SkASSERT(recorder); // Asserts when GM runs against PDF backend!
```

Good:

```cpp
// src/pdf/SkPDFDevice.h & src/xps/SkXPSDevice.h:
SkRecorder* baseRecorder() const override { return skcpu::Recorder::TODO(); }

// src/svg/SkSVGDevice.h:
SkRecorder* baseRecorder() const override { return skcpu::Recorder::TODO(); }
```
