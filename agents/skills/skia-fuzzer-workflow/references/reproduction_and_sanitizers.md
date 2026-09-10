# Reproducing Fuzz Bugs & Configuring Sanitizers

This guide details how to reproduce fuzzer bug reports from ClusterFuzz, OSS-Fuzz, or local
reports locally in Skia using sanitizers and the `fuzz` binary.

---

## 1. Hermetic Build Configuration

Fuzzers often trigger crashes that depend on system libraries (e.g. system FreeType versions or
system font managers). To achieve reliable and hermetic reproduction matching the OSS-Fuzz
environment, configure your build directory with hermetic flags.

> [!IMPORTANT]
> **Never overwrite an existing user build directory** (e.g. `out/Debug`). Always create a
> dedicated directory, such as `out/ASAN` or `out/FuzzDebug`.

### Recommended GN Configuration for ASAN

Generate the build files using `./bin/gn`:

```bash
./bin/gn gen -q out/ASAN --args='
  is_debug = true
  sanitize = "ASAN"
  skia_use_fontconfig = false
  skia_use_freetype = true
  skia_use_system_freetype2 = false
  skia_use_wuffs = true
  skia_enable_skottie = true
  skia_enable_fontmgr_custom_directory = false
  skia_enable_fontmgr_custom_embedded = false
  skia_enable_fontmgr_custom_empty = true
'
```

### Other Sanitizer Configurations

- **MSAN (MemorySanitizer)**: Detects uninitialized memory reads.
  ```bash
  ./bin/gn gen -q out/MSAN --args='
    is_debug = true
    sanitize = "MSAN"
    skia_use_fontconfig = false
  '
  ```
  *Note*: When running tests under MSAN, use `--nogpu` with `dm` to avoid driver noise.

- **UBSAN (UndefinedBehaviorSanitizer)**: Detects integer overflows, invalid enum values,
  alignment faults, and shift overflows.
  ```bash
  ./bin/gn gen -q out/UBSAN --args='
    is_debug = true
    sanitize = "UBSAN"
  '
  ```

- **Debug Non-Sanitizer**: For fast step-through debugging with GDB or LLDB when memory
  corruption isn't involved.
  ```bash
  ./bin/gn gen -q out/DebugFuzz --args='
    is_debug = true
    skia_use_fontconfig = false
    skia_enable_fontmgr_custom_empty = true
  '
  ```

---

## 2. Compiling the Fuzzer Tool

Compile the unified `fuzz` executable using Ninja:

```bash
ninja --quiet -C out/ASAN fuzz
```

If compilation fails due to missing dependencies, run:
```bash
python3 tools/git-sync-deps
```

---

## 3. Running `fuzz` Against the Testcase

The `fuzz` binary accepts minified testcase byte files directly via `-b` (or `--bytes`).

### Automatic Type Detection

```bash
out/ASAN/fuzz -b /path/to/testcase
```

The tool inspects the filename and header bytes to deduce the fuzzer type (e.g. filenames
containing `path_deserialize`, `sksl2glsl`, `skp`, `api_RasterN32Canvas`).

### Explicit Type and Name Specification

If automatic detection fails or the testcase filename is just a hash, specify the target type
with `-t` and optional API name with `-n`:

```bash
# Path deserialization
out/ASAN/fuzz -t path_deserialize -b /path/to/testcase

# Canvas API fuzzing
out/ASAN/fuzz -t api -n RasterN32Canvas -b /path/to/testcase

# SkSL translation
out/ASAN/fuzz -t sksl2glsl -b /path/to/testcase

# Image decoding
out/ASAN/fuzz -t image_decode -b /path/to/testcase

# Filter fuzz (Chrome filter_fuzz_stub equivalent)
out/ASAN/fuzz -t filter_fuzz -b /path/to/testcase
```

To list all supported fuzzer types and API names:
```bash
out/ASAN/fuzz --help
out/ASAN/fuzz -t api
```

### Intermittent Crashes and Loops

If a crash depends on internal caches, state reuse, or non-deterministic behavior, use `--loops`:

```bash
out/ASAN/fuzz -b /path/to/testcase --loops 50
```

---

## 4. Extracting Testcase Artifacts from Issue Trackers

- **ClusterFuzz / OSS-Fuzz**: Download the file labeled `minimized-testcase-...` or
  `clusterfuzz-testcase-...`.
- **Buganizer (Issue Tracker)**: Look for attachments ending in `.raw`, `.bytes`, `.skp`, `.gif`,
  `.png`, or `.json`.
- Store the testcase in a temporary scratch location or under your conversation artifact directory:
  `<appDataDir>/brain/<conversation-id>/scratch/testcase`
