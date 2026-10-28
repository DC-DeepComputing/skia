---
name: skia/build-gn/avoid-duplicate-public-and-all-dependent-configs
description: When configuring GN targets or templates, do not assign the same config to both public_configs and all_dependent_configs to avoid redundant configuration propagation
rubric: readability
severity: should_fix
---

**Rule.** In GN target definitions and templates, do not assign the same config
to both public_configs and all_dependent_configs. Choose all_dependent_configs
only when configuration settings must propagate transitively down the dependency
tree; otherwise, use public_configs for direct dependents.

**Why.** all_dependent_configs applies a config to the target and all transitive
dependents, which subsumes public_configs (which applies only to the target and
direct dependents). Specifying both creates redundant build graph configuration,
clutters target definitions, and obscures whether transitive propagation was
intentionally desired.

## Examples

Bad:

```cpp
skia_source_set(target_name) {
  public_configs = [ ":" + target_name + "_public" ]
  # Redundant: all_dependent_configs already applies to direct dependents
  all_dependent_configs = [ ":" + target_name + "_public" ]
}
```

Good:

```cpp
skia_source_set(target_name) {
  # Use only all_dependent_configs if transitive propagation is required
  all_dependent_configs = [ ":" + target_name + "_public" ]
}
```
