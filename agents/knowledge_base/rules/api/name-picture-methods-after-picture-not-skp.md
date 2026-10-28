---
name: skia/api/name-picture-methods-after-picture-not-skp
description: When naming methods or functions returning an in-memory SkPicture, use Picture rather than SKP, reserving SKP for serialized streams or file formats
rubric: readability
severity: nit
---

**Rule.** Do not name methods or functions that return an in-memory `SkPicture`
after `SKP` (e.g., `snapSKP()`); use `Picture` (e.g., `snapPicture()`,
`finishRecordingAsPicture()`) instead. Reserve `SKP` for serialized streams,
byte buffers, or file-format operations.

**Why.** In Skia, `SKP` designates the serialized stream or file format (`.skp`)
of an `SkPicture`. Using `SKP` for methods returning live, in-memory
`sk_sp<SkPicture>` objects conflates in-memory object lifetimes with serialized
storage and contradicts established API naming conventions across Skia.

## Examples

Bad:

```cpp
// Bad: Names method returning an in-memory SkPicture after serialized format SKP
sk_sp<SkPicture> snapSKP();
```

Good:

```cpp
// Good: Uses Picture for returning an in-memory SkPicture, reserving SKP for serialized formats
sk_sp<SkPicture> snapPicture();
```
