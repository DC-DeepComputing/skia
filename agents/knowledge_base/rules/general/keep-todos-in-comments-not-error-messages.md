---
name: skia/keep-todos-in-comments-not-error-messages
description: When emitting user-facing compiler errors, diagnostics, or log messages, describe unsupported features cleanly and keep TODO annotations or tracking bugs in code comments
rubric: readability
severity: nit
---

**Rule.** Do not include `TODO` markers, `FIXME` annotations, or issue tracker
references (such as `skia:1234` or `b/1234`) in user-visible compiler errors,
diagnostics, or log messages. State the unsupported construct or error condition
clearly for the end user in the diagnostic, and document planned tasks,
follow-up enhancements, and tracking bug references in source code comments
instead.

**Why.** User-visible compiler diagnostics and error messages are surfaced
directly to developers and end users. Embedding internal tracking markers, bug
numbers, and TODO tags in user-facing output clutters diagnostics and exposes
internal project management details, whereas tracking comments belong in source
code where maintainers can inspect and resolve them.

## Examples

Bad:

```cpp
fContext.fErrors->error(
        p.fPosition,
        "TODO(skia:14082): prefix '++' and '--' not yet supported in WGSL backend");
```

Good:

```cpp
// TODO(skia:14082): Prefix increment/decrement expressions should be transformed in IR.
fContext.fErrors->error(p.fPosition,
                        "prefix '++' and '--' not yet supported in WGSL backend");
```
