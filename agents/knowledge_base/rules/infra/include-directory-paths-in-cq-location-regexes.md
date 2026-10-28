---
name: skia/infra/include-directory-paths-in-cq-location-regexes
description: When configuring cq_config.location_regexes in infra/bots/jobs.json, explicitly list subsystem directories, scope tests to tests/<subsystem>/.* rather than tests/.*, and omit auto-rolled DEPS
rubric: correctness
severity: should_fix
---

**Rule.** When configuring Commit Queue trigger filters
(`cq_config.location_regexes`) in `infra/bots/jobs.json`, explicitly list
subsystem source and tool directories (e.g., `(include|src)/<subsystem>/.*`)
rather than CamelCase substring wildcards. Scope test filters for
subsystem-specific jobs to their respective test directories (e.g.,
`tests/graphite/.*` or `tests/ganesh/.*`) instead of top-level catch-alls like
`tests/.*`. For Graphite jobs, include precompiled SkSL modules
(`src/sksl/generated/.*`) rather than SkSL codegen paths (`sksl/codegen/.*`),
and include specific backend dependencies (such as `bazel/external/dawn/*`)
rather than generic runner regexes like `dm/.+`. Do not add coarse `"DEPS"`
triggers to jobs for dependencies already covered by auto-rollers.

**Why.** Overly broad test regexes like `tests/.*` trigger specialized, slow
backend CI jobs on presubmit for unrelated test changes across the repository.
Substring wildcards miss lowercase directory paths and companion test/tool
files, while targeting SkSL codegen instead of precompiled modules misses shader
changes. Coarse `DEPS` triggers or generic test runner regexes waste CI bot
capacity and lengthen CQ turnaround.

## Examples

Bad:

```cpp
// Top-level test catch-all and SkSL codegen instead of precompiled modules:
"name": "Test-Ubuntu24.04-Clang-GCE-GPU-SwiftShader-x86_64-Debug-All-SwiftShader_Graphite",
"cq_config": {
  "location_regexes": [
    "(include|src)/gpu/graphite/vk/.*",
    "(include|src)/sksl/codegen/.*",
    "tests/.*"
  ]
}

// Missing directory paths (CamelCase substring only) or auto-rolled DEPS:
"name": "Test-Win10-MSVC-Golo-GPU-QuadroP400-x86_64-Debug-All-Graphite_Dawn_D3D11",
"cq_config": {
  "location_regexes": ["(include|src)/gpu/graphite/.*Precompile.*", "dm/.+", "DEPS"]
}
```

Good:

```cpp
// Scoped subsystem test directory and precompiled SkSL modules:
"name": "Test-Ubuntu24.04-Clang-GCE-GPU-SwiftShader-x86_64-Debug-All-SwiftShader_Graphite",
"cq_config": {
  "location_regexes": [
    "(include|src)/gpu/graphite/vk/.*",
    "src/sksl/generated/.*",
    "tests/graphite/.*"
  ]
}

// Explicit directory paths, backend dependencies, and no auto-rolled DEPS:
"name": "Test-Win10-MSVC-Golo-GPU-QuadroP400-x86_64-Debug-All-Graphite_Dawn_D3D11",
"cq_config": {
  "location_regexes": [
    "(include|src)/gpu/graphite/precompile/.*",
    "(tests|tools)/graphite/precompile/.*",
    "bazel/external/dawn/*"
  ]
}
```
