---
name: skia/api/document-friend-declaration-purpose
description: When declaring friend classes or structs, document the specific target method, constructor, or member variable being granted access in an inline comment
rubric: readability
severity: should_fix
---

**Rule.** When declaring a friend class or struct to grant access to restricted
constructors, private methods, or internal state, document the specific target
member or reason with an inline comment (e.g., `// For <Name> constructor`, `//
for <method>`, or `// needs <field>`).

**Why.** Friend declarations grant coarse-grained access to all private and
protected members of a class. Explaining why friendship is required and which
specific members are accessed clarifies encapsulation boundaries, simplifies
security audits, and helps future refactorings determine whether the friendship
can be safely removed or narrowed.

## Examples

Bad:

```cpp
class UniformDataBlock {
private:
    friend class PipelineDataGatherer;

    constexpr UniformDataBlock(SkSpan<const char> data) : fData(data) {}
};
```

Good:

```cpp
class UniformDataBlock {
private:
    friend class PipelineDataGatherer;  // For SkSpan constructor

    constexpr UniformDataBlock(SkSpan<const char> data) : fData(data) {}
};
```
