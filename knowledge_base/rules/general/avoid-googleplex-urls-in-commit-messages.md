---
name: skia/avoid-googleplex-urls-in-commit-messages
description: When citing reviews or changes in commit messages, avoid internal corp URLs and deep patchset links; use canonical https://review.skia.org/<cl> and repo-relative paths
rubric: readability
severity: nit
---

**Rule.** Do not include internal Google corporate URLs (such as
`*.git.corp.google.com` or `googleplex.*`) or deep Gerrit patch-set file diff
links in Git commit messages. When linking to Skia code reviews, use the public
canonical URL `https://review.skia.org/<cl>` (or
`https://skia-review.googlesource.com/c/skia/+/<cl>`), referencing specific
files by their repository-relative paths instead of deep URLs. When citing
internal Android Gerrit changes, use shortlinks like `http://ag/<change_id>`
instead.

**Why.** Skia is an open-source project with a public git history; internal
corporate hostnames require internal credentials and are inaccessible to
external contributors, whereas public canonical URLs and http://ag/<id>
shortlinks remain accessible. Deep Gerrit links to specific patch-set file diffs
are fragile, prone to broken links or permission issues, and clutter git
history, whereas referencing the canonical CL URL and naming repository-relative
file paths provides durable, readable context in git log.

## Examples

Bad:

```cpp
adapted from abandoned dry roll https://skia-review.git.corp.google.com/c/skia/+/1194816
Reason for revert: Can now land after https://googleplex-android-review.git.corp.google.com/c/platform/frameworks/native/+/38975924
was accidentally included in https://review.skia.org/1159676
(https://review.skia.org/c/skia/+/1159676/22/src/gpu/ganesh/d3d/GrD3DBackendSurfacePriv.h was correct).
```

Good:

```cpp
adapted from abandoned dry roll https://review.skia.org/1194816
Reason for revert: Can now land after http://ag/38975924
was accidentally included in https://review.skia.org/1159676
(src/gpu/ganesh/d3d/GrD3DBackendSurfacePriv.h was correct).
```
