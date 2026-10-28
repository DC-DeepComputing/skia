---
name: skia/presubmit/normalize-presubmit-localpath-with-np
description: When matching LocalPath() against forward-slash paths in PRESUBMIT.py, normalize the path using np() to ensure cross-platform compatibility on Windows
rubric: correctness
severity: should_fix
---

**Rule.** In `PRESUBMIT.py`, normalize file paths returned by
`affected_file.LocalPath()` using the `np()` helper before matching against path
prefixes, suffixes, or substrings containing forward slashes (`/`). Do not
compare raw `LocalPath()` strings directly against forward-slash paths (such as
`x.LocalPath().startswith('include/')`).

**Why.** On Windows, `AffectedFile.LocalPath()` produces backslash-separated
paths (`\`). Direct comparisons against forward-slash path literals silently
fail on Windows machines, causing presubmit filters and checks to be bypassed.

## Examples

Bad:

```cpp
file_filter = lambda x: x.LocalPath().startswith('include/')
```

Good:

```cpp
file_filter = lambda x: np(x.LocalPath()).startswith('include/')
```
