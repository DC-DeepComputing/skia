---
name: skia/keep-depfile-before-ninja-tdeps
description: When compiling third-party dependencies with Ninja in custom build scripts, specify explicit target names and compile with -dkeepdepfile before querying depfiles
rubric: safety
severity: should_fix
---

**Rule.** In custom build scripts that invoke Ninja to compile third-party
dependencies, explicitly specify the required target names rather than invoking
a generic build-all command. Additionally, when generating GN depfiles (.d),
execute the Ninja build with '-dkeepdepfile' before querying dependencies, and
extract header dependencies using 'ninja -tdeps' rather than inspecting static
inputs beforehand.

**Why.** Compiling an entire third-party repository without explicit target
filters builds unnecessary libraries and tests, creating substantial build-time
overhead. Furthermore, Ninja only records implicit header dependencies
dynamically during compilation when compiler depfiles are generated; passing
'-dkeepdepfile' preserves those depfiles so 'ninja -tdeps' can accurately record
dependencies for incremental rebuilds.

## Examples

Bad:

```cpp
# Build default target without explicit targets, and query deps before compiling
dependencies = discover_dependencies(build_dir, tint_targets)
write_depfile(output_path, depfile_path, dependencies)

build_cmd = [ninja_exe, "-C", build_dir]
subprocess.run(build_cmd, check=True)
```

Good:

```cpp
# Explicitly specify the required targets
tint_targets = ["tint_api", "tint_lang_wgsl_reader", "tint_lang_wgsl_writer"]

# Build first with -dkeepdepfile so compiler depfiles are generated and retained
build_cmd = [ninja_exe, "-C", build_dir, "-dkeepdepfile"] + tint_targets
subprocess.run(build_cmd, check=True)

# Discover dependencies after compilation using ninja -tdeps
dependencies = discover_dependencies(build_dir, tint_targets)
write_depfile(output_path, depfile_path, dependencies)
```
