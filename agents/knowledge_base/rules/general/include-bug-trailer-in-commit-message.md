---
name: skia/include-bug-trailer-in-commit-message
description: 'When authoring commit footers, separate resolved issues into Fixed: and tracking bugs into Bug: before Change-Id, using complete issue IDs'
rubric: readability
severity: should_fix
---

**Rule.** When authoring or updating commit messages, include capitalized `Bug:`
or `Fixed:` (never lowercase `bug:`) trailers referencing all associated issue
tracker IDs for fixes, related issues, or test suppressions (or `None` if no
issue exists) in the footer paragraph preceding `Change-Id:`. Separate resolved
issues into a dedicated `Fixed:` line (to auto-close the issue on landing) and
use `Bug:` for ongoing tracking, related, or suppression issues that should
remain open. Place each issue identifier on its own separate trailer line rather
than combining multiple IDs into a comma- or semicolon-separated list. Format
bug references using complete issue IDs rather than bare tracker prefixes (e.g.,
avoid `Bug: chromium:`) or full URLs; for Chromium issues, use `chromium:<id>`
instead of `b/<id>` to link directly to the public tracker. Ensure `Change-Id:`
is always the final trailer line.

**Why.** Gerrit and repository automation differentiate between resolved issues
and ongoing tracking issues: `Fixed:` automatically closes the issue upon
landing, whereas `Bug:` links the change without closing it prematurely. Parsers
inspect metadata footers case-sensitively (lowercase `bug:` fails parser
matching), require each issue on its own trailer line to avoid hyperlink
corruption from comma-separated lists, and require `Change-Id:` to remain last
to prevent depot_tools parsing errors. Using `chromium:<id>` links directly to
the public Chromium tracker without internal redirect hops.

## Examples

Bad:

```cpp
// Combining multiple bug IDs on a single line or using Bug: for resolved issues:
Bug: b/413427423, 414609517
Change-Id: I2349f7c685ab62b377823f34b628f0234b4dab26

// Lowercase bug trailer (Gerrit parser ignores lowercase):
bug: 447041289
Change-Id: I578aaa77ce58549b83c8b8d97c2d8c5d3c735f6e

// Using internal b/ redirect for Chromium issues, incomplete prefixes, or placing after Change-Id:
Change-Id: Ic97ffafaf5c49f107f6ae8e2caa2659c08f09432
Bug: b/359245096
Bug: chromium:
```

Good:

```cpp
// Separating resolved issues into Fixed: and tracking bugs into Bug: on individual lines:
Bug: 413427423
Bug: 414594133
Fixed: 414609517
Change-Id: I2349f7c685ab62b377823f34b628f0234b4dab26

// Capitalized single-issue Bug: trailer placed before Change-Id:
Bug: 447041289
Change-Id: I578aaa77ce58549b83c8b8d97c2d8c5d3c735f6e

// Using chromium:<id> for Chromium bugs, placed before Change-Id:
Bug: chromium:359245096
Bug: chromium:493179864
Change-Id: Ic97ffafaf5c49f107f6ae8e2caa2659c08f09432
```
