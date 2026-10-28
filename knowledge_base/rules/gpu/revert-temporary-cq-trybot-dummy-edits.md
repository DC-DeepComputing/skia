---
name: skia/gpu/revert-temporary-cq-trybot-dummy-edits
description: When modifying files, do not commit temporary dummy edits or unrelated whitespace changes used to trigger CQ trybots; revert them before landing
rubric: design
severity: should_fix
---

**Rule.** Do not leave temporary dummy edits, artificial comments, or unrelated
whitespace changes used to trigger CQ trybots in production or test files.
Revert all temporary troubleshooting and trybot-triggering modifications before
landing, or trigger required trybots directly using `git cl try` or
`Cq-Include-Trybots` in the commit message.

**Why.** Dummy edits and stray whitespace introduced solely to satisfy CQ path
filters create review noise, obscure meaningful code changes, and pollute git
commit history. Using official trybot invocation mechanisms (`git cl try` or
`Cq-Include-Trybots`) keeps changesets clean and focused.

## Examples

Bad:

```cpp
// In src/gpu/graphite/Texture.cpp (modified solely to trigger (tests|src/gpu)/graphite/.* trybots):
#include "src/gpu/graphite/Texture.h"



#include "include/core/SkTraceMemoryDump.h"
```

Good:

```cpp
// Trigger trybots in COMMIT_MSG instead of adding dummy edits to source files:
// Cq-Include-Trybots: skia.primary:Test-Mac12-Clang-MacBookPro16.2-GPU-IntelIrisPlus-x86_64-Debug-All-ANGLE

#include "src/gpu/graphite/Texture.h"

#include "include/core/SkTraceMemoryDump.h"
```
