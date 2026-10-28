---
name: skia/readme-chromium/specify-autoroll-update-mechanism-in-readme-chromium
description: When adding or modifying README.chromium metadata files for repositories or modules rolled by bots, set Update Mechanism to Autoroll rather than Manual
rubric: readability
severity: should_fix
---

**Rule.** In README.chromium metadata files, specify 'Update Mechanism:
Autoroll' rather than 'Manual' when the repository or component is updated in
Chromium via an automated roll bot. Ensure the third-party metadata accurately
reflects the project's automated roll integration.

**Why.** Skia and its vendored submodules (such as skcms) are integrated into
Chromium via automated roll bots rather than manual merges. Specifying
'Autoroll' satisfies Chromium third-party metadata conventions and prevents
tooling or maintainers from assuming manual update procedures.

## Examples

Bad:

```cpp
Name: Skia
URL: https://skia.org/
Version: N/A
Revision: DEPS
Update Mechanism: Manual
```

Good:

```cpp
Name: Skia
URL: https://skia.org/
Version: N/A
Revision: DEPS
Update Mechanism: Autoroll
```
