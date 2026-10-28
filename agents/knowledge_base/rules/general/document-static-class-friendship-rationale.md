---
name: skia/document-static-class-friendship-rationale
description: When defining a static-only utility or accessor class instead of a namespace to allow friendship, document that friendship rationale in a header comment
rubric: readability
severity: nit
---

**Rule.** When defining a static-only utility class, internal bridge, or
accessor class (such as a *Priv or *Accessor class) instead of a namespace to
access private members of other types, add a comment documenting that a class is
used so target types can friend it. Delete default and copy constructors to
enforce non-constructibility.

**Why.** In modern C++ and Skia style conventions, stateless utility functions
belong in namespaces rather than non-constructible classes. However, C++ does
not allow declaring friendship with a namespace. Explaining that a static-only
class exists specifically to enable friendship clarifies the architectural
intent and prevents reviewers from requesting conversion to a namespace.

## Examples

Bad:

```cpp
class TextureInfoPriv {
public:
    static TextureFormat ViewFormat(const TextureInfo& info);

private:
    TextureInfoPriv() = delete;
    TextureInfoPriv(const TextureInfoPriv&) = delete;
};
```

Good:

```cpp
// NOTE: This is a class so that it can be friended by TextureInfo and the backend info classes.
class TextureInfoPriv {
public:
    static TextureFormat ViewFormat(const TextureInfo& info);

private:
    TextureInfoPriv() = delete;
    TextureInfoPriv(const TextureInfoPriv&) = delete;
};
```
