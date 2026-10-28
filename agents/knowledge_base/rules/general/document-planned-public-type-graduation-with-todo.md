---
name: skia/document-planned-public-type-graduation-with-todo
description: When introducing incubating classes or types in internal headers under src/ that are planned for public API exposure, document the intended graduation with an attributed TODO comment
rubric: readability
severity: nit
---

**Rule.** When introducing a class, struct, or interface type in an internal
header under src/ that is slated to graduate to the public API (under include/),
document the planned graduation with an explanatory comment or attributed
TODO(b/...) / TODO(skbug.com/...) comment directly above the type declaration.
State the tracking bug and intended public exposure so contributors do not
introduce tight couplings to private internals or question the public naming
conventions.

**Why.** Headers in src/ are private and excluded from client search paths.
Incubating new public API types in internal headers while in progress prevents
premature public API commitments, but without explicit documentation,
maintainers may tightly couple the type to private internals or misunderstand
why it uses public naming conventions.

## Examples

Bad:

```cpp
// src/capture/SkCapture.h
class SkCapture : public SkRefCnt {
public:
    static sk_sp<SkCapture> MakeFromData(sk_sp<SkData> data);
    ...
};
```

Good:

```cpp
// src/capture/SkCapture.h
// TODO(b/412351769): Make this class public in include/core/SkCapture.h.
class SkCapture : public SkRefCnt {
public:
    static sk_sp<SkCapture> MakeFromData(sk_sp<SkData> data);
    ...
};
```
