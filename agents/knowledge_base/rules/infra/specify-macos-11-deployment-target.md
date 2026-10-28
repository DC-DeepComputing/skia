---
name: skia/infra/specify-macos-11-deployment-target
description: When configuring macOS deployment targets or target triples for macOS 11 (Big Sur), specify '11.0' rather than '10.11' to avoid lowering the target baseline
rubric: correctness
severity: should_fix
---

**Rule.** When setting macOS deployment targets (such as
MACOSX_DEPLOYMENT_TARGET or compiler target flags), specify macOS 11 as '11.0'
(or '11') rather than '10.11'. Remember that macOS versioning transitioned
directly from 10.15 (Catalina) to 11.0 (Big Sur).

**Why.** Specifying '10.11' refers to OS X 10.11 (El Capitan), which
inadvertently lowers the minimum OS deployment target rather than raising it to
macOS 11 (Big Sur).

## Examples

Bad:

```cpp
env['MACOSX_DEPLOYMENT_TARGET'] = '10.11'
```

Good:

```cpp
env['MACOSX_DEPLOYMENT_TARGET'] = '11.0'
```
