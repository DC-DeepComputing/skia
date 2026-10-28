---
name: skia/match-deserialized-variable-names-to-fields
description: When deserializing data from buffers into local variables to construct objects, name variables to match the target member fields and constructor parameters
rubric: readability
severity: should_fix
---

**Rule.** In buffer deserialization routines (such as MakeFromBuffer or
unflatten), name local variables that store decoded values after the
corresponding constructor parameter, member field, or serialized property. Avoid
diverging, misleading, or inverted names (such as using 'write' prefixes or
verbs in a buffer read routine) that do not match the target object's
properties.

**Why.** Aligning local variable names with destination constructor parameters
and member fields ensures consistency between serialization and deserialization
routines, simplifies code auditing, and prevents confusion caused by misleading
identifiers.

## Examples

Bad:

```cpp
// Local variable name diverges from constructor parameter and member field:
const bool canWriteDirect = buffer.readBool();
return TransformedMaskVertexFiller{
    maskType, creationMatrix, creationBounds, leftTop, canWriteDirect};
```

Good:

```cpp
// Local variable matches constructor parameter and member field:
const bool canDrawDirect = buffer.readBool();
return TransformedMaskVertexFiller{
    maskType, creationMatrix, creationBounds, leftTop, canDrawDirect};
```
