---
name: skia/gpu/serialize-new-caps-in-dumpjson
description: When adding new capability, feature, or workaround flags to GrCaps or backend subclasses, update dumpJSON or onDumpJSON to serialize the flag
rubric: correctness
severity: should_fix
---

**Rule.** When adding a new capability, workaround, or driver feature flag to
GrCaps or its backend subclasses (such as GrGLCaps, GrShaderCaps, or GrMtlCaps),
update dumpJSON (or backend onDumpJSON) to serialize the flag using SkJSONWriter
(e.g., writer->appendBool). Ensure newly introduced boolean flags, feature
limits, and driver workaround states are reflected in JSON diagnostics.

**Why.** Ganesh capability classes provide dumpJSON and onDumpJSON to export GPU
capabilities, limits, and driver workaround states for debugging tools, crash
reporting, and diagnostic logging. Omitting newly added capability fields leaves
diagnostic dumps out of date and prevents inspection tools from accurately
reflecting the active driver configuration.

## Examples

Bad:

```cpp
// In src/gpu/ganesh/GrCaps.h / GrCaps.cpp:
fDisableTessellationPathRenderer = false;
fAvoidStencilBuffers = true;

// GrCaps::dumpJSON() is left unchanged, omitting the new capability flags.
```

Good:

```cpp
// In src/gpu/ganesh/GrCaps.cpp dumpJSON(SkJSONWriter* writer):
writer->appendBool("Disable TessellationPathRenderer current driver [workaround]",
                   fDisableTessellationPathRenderer);
writer->appendBool("Avoid stencil buffers [workaround]", fAvoidStencilBuffers);
```
