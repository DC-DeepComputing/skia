---
name: skia/gpu/check-native-and-angle-drivers-separately-in-gl-caps
description: When implementing driver workarounds in Ganesh GL caps, check native and ANGLE drivers separately at workaround sites instead of redirecting GrGLContext::driverVersion
rubric: design
severity: blocking
---

**Rule.** When adding or modifying driver workarounds in Ganesh GL capabilities
(GrGLCaps), do not redirect or alias GrGLContext::driverVersion() to return the
ANGLE driver version. Instead, leave driverVersion() querying the native OpenGL
driver and explicitly evaluate both native driver properties (driver(),
driverVersion()) and ANGLE driver properties (angleBackend(), angleDriver(),
angleDriverVersion()) at specific workaround call sites.

**Why.** Caps workaround predicates inspect combinations of vendor, driver,
renderer, and driver version that behave differently when ANGLE translates GL
commands to another backend. Centrally overriding driverVersion() causes subtle
mismatches with other driver and vendor properties across unrelated workarounds,
whereas explicit call-site checks ensure each workaround applies only to the
exact native or ANGLE driver configurations affected by the bug.

## Examples

Bad:

```cpp
// In GrGLContext.h:
GrGLDriverVersion driverVersion() const {
    return (this->angleBackend() == GrGLANGLEBackend::kUnknown)
        ? fDriverInfo.fDriverVersion
        : fDriverInfo.fANGLEDriverVersion;
}
```

Good:

```cpp
// At individual workaround call sites in GrGLCaps.cpp:
if ((ctxInfo.driver() == GrGLDriver::kNVIDIA &&
     ctxInfo.driverVersion() < GR_GL_DRIVER_VER(355, 00, 0)) ||
    (ctxInfo.angleBackend() == GrGLANGLEBackend::kOpenGL &&
     ctxInfo.angleDriver() == GrGLDriver::kNVIDIA &&
     ctxInfo.angleDriverVersion() < GR_GL_DRIVER_VER(355, 00, 0))) {
    // apply workaround
}
```
