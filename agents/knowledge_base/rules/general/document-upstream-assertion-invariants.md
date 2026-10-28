---
name: skia/document-upstream-assertion-invariants
description: When writing assertions relying on caller preconditions, class invariants, or external/FFI validation, document with inline comments where the invariant is guaranteed
rubric: readability
severity: nit
---

**Rule.** When writing assertions (such as SkASSERT or SkASSERT_RELEASE) whose
invariants rely on external libraries, FFI boundaries, caller preconditions, or
non-local class invariants (such as state established in a constructor), add an
inline comment explaining why the condition is guaranteed to hold and who
enforces it. Avoid leaving unannotated assertions on data properties, buffer
sizes, or parser state that appear to be crash hazards or missing runtime checks
on untrusted input.

**Why.** In code parsing untrusted input or invoking FFI functions, assertions
on buffer bounds, chunk sizes, or object state look like crash hazards or
missing runtime error checks if readers cannot tell whether verification was
inadvertently omitted. Documenting which upstream function, crate, caller
contract, or constructor invariant guarantees the condition clarifies why
runtime checks are unnecessary and prevents reviewer confusion.

## Examples

Bad:

```cpp
// Bad: Unclear whether external data was pre-validated or if verification was omitted.
SkASSERT_RELEASE(sBit.size() == 3);

SkASSERT(reader.has_fctl_chunk());
SkASSERT(!fFrames.empty());
reader.get_fctl_info(...);
```

Good:

```cpp
// Good: Inline comments explain where the invariant or precondition is established.
SkASSERT_RELEASE(sBit.size() == 3);  // Verified in `png` crate in `fn parse_sbit`.

SkASSERT(reader.has_fctl_chunk());   // Caller should guarantee this
SkASSERT(!fFrames.empty());          // SkPngRustCodec's invariant (see constructor)
reader.get_fctl_info(...);
```
