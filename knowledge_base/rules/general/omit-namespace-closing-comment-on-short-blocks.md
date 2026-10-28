---
name: skia/omit-namespace-closing-comment-on-short-blocks
description: When terminating C++ namespaces, annotate anonymous namespaces with `// anonymous namespace` preceded by a blank line, repeat named namespaces, and omit comments on short blocks
rubric: readability
severity: nit
---

**Rule.** When terminating C++ namespaces, annotate the closing brace of an
anonymous namespace with `// anonymous namespace` (never bare `// namespace`)
preceded by a blank line to separate it cleanly from enclosing definitions. For
named namespaces, repeat the namespace name (e.g., `} // namespace Foo`) on
multi-line blocks and never use a bare `// namespace`. Omit closing comments
entirely on short forward-declaration blocks.

**Why.** An unannotated closing brace placed immediately after a function or
class definition can easily be mistaken for function or class termination.
Repeating the namespace name on non-trivial named blocks clarifies the enclosing
scope across long spans, while bare `// namespace` is ambiguous and fails to
distinguish anonymous namespaces from named namespaces. Conversely, closing
comments on short forward-declaration blocks add visual noise without navigation
benefit.

## Examples

Bad:

```cpp
// Bad: unannotated anonymous namespace brace immediately following a function
    return targetSize;
}
}

// Bad: bare '// namespace' on anonymous namespace
}  // namespace

// Bad: bare '// namespace' on named namespace
namespace SkCodecs {
class ColorProfile;
}  // namespace

// Bad: unnecessary closing comment on a short forward-declaration scope
namespace skgpu::graphite {
class Recorder;
}  // namespace skgpu::graphite
```

Good:

```cpp
// Good: blank line and explicit '// anonymous namespace' comment
    return targetSize;
}

}  // anonymous namespace

// Good: omit closing comments on short forward-declaration blocks
namespace SkCodecs {
class ColorProfile;
}

// Good: repeat the namespace name on multi-line definitions
namespace SkCodecs {

SK_API std::unique_ptr<SkCodec> MakeFromStream(...);

}  // namespace SkCodecs
```
