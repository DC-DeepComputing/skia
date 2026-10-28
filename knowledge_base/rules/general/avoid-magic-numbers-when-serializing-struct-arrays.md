---
name: skia/avoid-magic-numbers-when-serializing-struct-arrays
description: When serializing or deserializing structs as scalar or byte arrays via SkWriteBuffer and SkReadBuffer, compute element counts statically instead of hardcoding magic numbers
rubric: readability
severity: should_fix
---

**Rule.** When serializing or deserializing structs as scalar or byte arrays via
SkWriteBuffer and SkReadBuffer (such as via writeScalarArray or
readScalarArray), compute element counts statically using sizeof (e.g.,
sizeof(T) / sizeof(SkScalar)) or share a named constexpr constant. Do not
hardcode raw numeric literals for struct element counts across flatten() and
deserialization factory procedures.

**Why.** Hardcoding numeric literals for struct element counts risks
reader-writer desynchronization and out-of-bounds reads or writes if the struct
definition changes or contains unexpected padding. Statically computing the
count or defining a shared constant guarantees that serialization and
deserialization remain in sync and match the underlying struct size.

## Examples

Bad:

```cpp
// In flatten():
buffer.writeScalarArray({&fTF.g, 7});

// In CreateProc():
buffer.readScalarArray({&tf.g, 7});
```

Good:

```cpp
// In flatten():
buffer.writeScalarArray({&fTF.g, sizeof(skcms_TransferFunction) / sizeof(SkScalar)});

// In CreateProc():
buffer.readScalarArray({&tf.g, sizeof(skcms_TransferFunction) / sizeof(SkScalar)});
```
