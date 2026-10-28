---
name: skia/build-gn/propagate-feature-configs-consistently
description: When configuring shared feature configs across parallel GN targets like Ganesh and Graphite, propagate them consistently via public_configs rather than private configs
rubric: correctness
severity: should_fix
---

**Rule.** In GN build files, when configuring feature configs or defines
intended for downstream targets, apply them consistently via 'public_configs'
across all parallel backend targets (such as Ganesh and Graphite in BUILD.gn),
rather than using private 'configs' in one and 'public_configs' in another.
Verify that standalone configurations enabling only one backend export all
necessary defines and header configurations required by dependent targets.

**Why.** GN's 'configs' applies settings only locally to the target, whereas
'public_configs' propagates them to downstream dependent targets. When parallel
backend targets share a feature, making a config private in one backend silently
succeeds in joint builds where the other backend exports it publicly, but causes
compilation or missing-define breakages in standalone builds where only the
former backend is enabled.

## Examples

Bad:

```cpp
optional("graphite") {
  ...
  if (skia_use_backup_vma) {
    # Private configs do not propagate to downstream consumers
    configs += [ ":use_backup_vma" ]
  }
}

optional("gpu") {
  ...
  if (skia_use_backup_vma) {
    public_configs += [ ":use_backup_vma" ]
  }
}
```

Good:

```cpp
optional("graphite") {
  ...
  if (skia_use_backup_vma) {
    # Consistently export via public_configs across both backends
    public_configs += [ ":use_backup_vma" ]
  }
}

optional("gpu") {
  ...
  if (skia_use_backup_vma) {
    public_configs += [ ":use_backup_vma" ]
  }
}
```
