---
name: skia/use-google-llc-copyright-notice
description: When adding or modifying source files, place a single copyright notice at the top, avoid duplicate mid-file headers, and preserve original creation years
rubric: readability
severity: should_fix
---

**Rule.** When authoring new files in Skia, place a single copyright and BSD
license header attributing copyright to 'Google LLC' with the current calendar
year at the very top of the file, preceding `#pragma once`, header guards
(`#ifndef`), and `#include` directives. Do not omit the header, duplicate
license blocks mid-file or inside conditional compilation blocks, use legacy
'Google Inc.', copy stale years, or use external conventions like 'The Chromium
Authors'. On existing files, do not modify or bump the year or entity in
existing copyright notices during refactorings. When vendoring third-party
files, preserve upstream notices at the top before header guards.

**Why.** Placing a single copyright notice at the very top maintains consistent
repository file structure and ensures legal attribution is immediately visible
before preprocessor directives or code. A single top-level header satisfies
legal requirements, whereas duplicate mid-file or conditional license blocks are
redundant copy-paste boilerplate. The copyright year permanently records file
creation and must not be bumped on existing files, while new Skia files require
current-year 'Google LLC' attribution and vendored files require preserving
upstream notices.

## Examples

Bad:

```cpp
// Duplicate license block inside a conditional compilation block:
/*
 * Copyright 2012 The Android Open Source Project
 */
#if defined(SK_USE_LEGACY_CONVOLUTION_IMAGEFILTER)
/*
 * Copyright 2012 The Android Open Source Project
 */

// Stale year copied into a new file, or bumping year on existing file:
/*
 * Copyright 2023 Google LLC
 */
```

Good:

```cpp
// Single license header at top; conditional block contains only code:
/*
 * Copyright 2012 The Android Open Source Project
 */
#if defined(SK_USE_LEGACY_CONVOLUTION_IMAGEFILTER)
#include "include/core/SkAlphaType.h"
#endif

// New file uses current year; existing file preserves original creation year:
/*
 * Copyright 2026 Google LLC
 */
```
