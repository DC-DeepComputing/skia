---
name: skia/api/name-callback-aliases-after-domain-artifact
description: When defining type aliases for callable hooks, use a Callback suffix describing the domain artifact rather than Handler or vague container names
rubric: readability
severity: should_fix
---

**Rule.** When defining type aliases for callable hooks, visitor functions, or
event listeners, suffix the alias with 'Callback' (or 'Proc' for C-style
function pointers) rather than object-oriented suffixes like 'Handler'.
Furthermore, name the alias after the concrete domain artifact or event produced
(such as 'ShapedTextCallback') rather than using vague container nouns like
'Blob' or 'Data'.

**Why.** In Skia, 'Handler' typically denotes a multi-method object interface or
class, whereas 'Callback' or 'Proc' explicitly signals a callable intended for
function invocation. Additionally, generic container terms like 'Blob' or 'Data'
are ambiguous across subsystems that process multiple buffer or blob types (such
as text blobs vs. binary buffers); describing the specific domain artifact
clarifies dataflow and caller intent.

## Examples

Bad:

```cpp
// Ambiguous container name ('Blob') and object-style suffix ('Handler') for a callable
class BlobHandler;

using BlobHandler = std::function<void(const SkSVGRenderContext&,
                                       const sk_sp<SkTextBlob>&,
                                       const SkPaint*)>;
```

Good:

```cpp
// Explicitly identifies domain artifact ('ShapedText') and callable nature ('Callback')
using ShapedTextCallback = std::function<void(const SkSVGRenderContext&,
                                              const sk_sp<SkTextBlob>&,
                                              const SkPaint*)>;
```
