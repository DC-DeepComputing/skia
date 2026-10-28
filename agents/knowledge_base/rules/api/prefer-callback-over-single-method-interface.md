---
name: skia/api/prefer-callback-over-single-method-interface
description: When designing APIs that take a single action or accumulate local state, prefer callback types over single-method abstract interface classes
rubric: design
severity: should_fix
---

**Rule.** Prefer callback types (such as std::function or templated callable
parameters) over single-method abstract interface classes when an API only
requires callers to supply a single action or accumulate local state across
invocations. Avoid defining pure-virtual handler or visitor interfaces (such as
classes with only virtual void operator()(...) = 0) that force callers to
implement boilerplate subclasses and member variables just to maintain state.

**Why.** Defining an abstract interface class requires callers to write subclass
boilerplate, allocate or manage class instances, and store captured variables in
member fields just to accumulate state across invocations. Callback types allow
callers to pass concise inline lambdas with reference captures, mutating local
variables directly in caller scope without unnecessary class hierarchies or
virtual dispatch overhead.

## Examples

Bad:

```cpp
// Bad: Single-method abstract class forces callers to declare subclass boilerplate to hold state
class BlobHandler {
public:
    virtual ~BlobHandler() = default;
    virtual void operator()(const SkSVGRenderContext&, const sk_sp<SkTextBlob>&,
                            const SkPaint* fill, const SkPaint* stroke) = 0;
};

void ProcessBlobs(const SkSVGTextContext&, BlobHandler* handler);
```

Good:

```cpp
// Good: Callback type allows callers to pass inline lambdas with captures directly
using ShapedTextCallback = std::function<void(const SkSVGRenderContext&,
                                              const sk_sp<SkTextBlob>&,
                                              const SkPaint* fill,
                                              const SkPaint* stroke)>;

void ProcessBlobs(const SkSVGTextContext&, const ShapedTextCallback& callback);

// Call site accumulates state concisely via reference capture:
SkRect bounds = SkRect::MakeEmpty();
ProcessBlobs(ctx, [&bounds](const auto&, const auto& blob, auto*, auto*) {
    bounds.join(blob->bounds());
});
```
