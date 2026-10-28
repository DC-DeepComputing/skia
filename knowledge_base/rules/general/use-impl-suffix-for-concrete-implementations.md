---
name: skia/use-impl-suffix-for-concrete-implementations
description: When naming internal classes, use Impl for concrete interface implementations, reserve _Base for extensible base classes, and reserve Priv for companion accessor classes
rubric: readability
severity: should_fix
---

**Rule.** When defining concrete internal implementations of public or abstract
interfaces, suffix the class name with Impl (e.g., ParagraphImpl, SlugImpl)
rather than _Base. Reserve _Base (or Base) strictly for internal base classes
intended for further subclassing; never use _Base on final classes or types not
designed for inheritance. Correspondingly, reserve the Priv suffix exclusively
for companion attorney or flyweight accessor classes (such as SkCanvasPriv,
SkPicturePriv) that expose private members of a companion type; do not use Priv
to name general internal classes or simply to disambiguate an internal type from
an existing or upcoming public class.

**Why.** In Skia conventions, the _Base suffix (such as SkImage_Base or
SkSurface_Base) denotes internal base classes intended for further subclassing;
naming a final class with _Base is contradictory. Similarly, Priv is strictly
reserved for peer/attorney accessor classes; using Priv on unrelated internal
classes misleadingly implies they expose private members of a companion class,
while Impl or a distinct descriptive name clearly communicates concrete type
semantics.

## Examples

Bad:

```cpp
// Bad: Concrete final implementation named with _Base suffix
class SkRasterContext_Base final : public SkRasterContext {
public:
    // ...
};

// Bad: Suffixing an internal class with Priv merely to disambiguate from SkRecorder
class SkRecorderPriv final : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    // ...
};
```

Good:

```cpp
// Good: Concrete implementation suffixed with Impl; _Base reserved for base classes
class SkRasterContextImpl final : public SkRasterContext {
public:
    // ...
};

// Good: Internal class given a distinct, descriptive name; Priv reserved for accessors
class SkRecordCanvas final : public SkCanvasVirtualEnforcer<SkNoDrawCanvas> {
public:
    // ...
};
```
