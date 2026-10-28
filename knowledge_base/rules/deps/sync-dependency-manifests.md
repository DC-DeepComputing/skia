---
name: skia/deps/sync-dependency-manifests
description: When rolling dependencies or maintaining third-party roll scripts, synchronize manifests (DEPS, Bazel deps.json, deps_gen.go) and regenerate downstream patches and configs
rubric: correctness
severity: should_fix
---

**Rule.** When rolling or updating third-party dependencies (or writing and
maintaining dependency roll scripts), synchronize all dependency manifests in
the same CL: update DEPS, update Bazel dependencies (bazel/deps.json via
bazelisk run //bazel/deps_parser), and update bot dependencies
(infra/bots/deps/deps_gen.go via go run infra/bots/deps/generate.go).
Additionally, when dependencies rely on generated configuration headers or
downstream build patches (e.g., bazel/external/<dep>/*.patch), ensure roll
scripts regenerate and stage those artifacts (e.g., via
tools/generate_patches.py).

**Why.** Skia pins external dependencies across DEPS, bazel/deps.json, and
deps_gen.go for bots, while downstream build targets may depend on generated
patches and config headers. Omitting manifest updates breaks CI bots and Bazel
builds, and failing to automate patch and config regeneration in roll scripts
causes downstream builds to fall out of sync during dependency rolls.

## Examples

Bad:

```cpp
# third_party/expat/roll-expat.sh
# Omits bot manifest synchronization and downstream patch regeneration:
rolldeps "$@" &&
rollbazel &&
update_expat_config_h &&
commit
```

Good:

```cpp
# third_party/expat/roll-expat.sh
# Synchronizes all manifests and regenerates downstream Bazel patch files:
rolldeps "$@" &&
rollbazel &&
rolldepsgen &&
update_expat_config_h &&
update_bazel_patch &&
commit
```
