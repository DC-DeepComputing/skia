# Graphite capture examples: handy commands

All commands are run from the Skia root (e.g. `chromium/src/third_party/skia`).
`example/external_client` is its own Bazel module, so Bazel must be invoked from
inside that directory (hence the `cd` subshells below).

Requires [Bazelisk](https://github.com/bazelbuild/bazelisk) (`brew install bazelisk`)
and Xcode / Command Line Tools. Bazel 8.2.1 is pinned by `.bazelversion`.

## Targets

| Target | Source | Notes |
|---|---|---|
| `//:use_graphite_metal_capture` | `src/graphite_metal_capture.cpp` | Graphite native Metal backend |
| `//:use_graphite_dawn_capture` | `src/graphite_dawn_capture.cpp` | Graphite Dawn backend, `--adapter=metal\|webgpu` |

## Build

```sh
(cd example/external_client && bazelisk build //:use_graphite_dawn_capture)
(cd example/external_client && bazelisk build //:use_graphite_metal_capture)
```

Dawn's WebGPU-on-WebGPU backend (GN equivalent: `dawn_enable_webgpu_on_webgpu = true`)
is enabled by default in `.bazelrc`. To build without it:

```sh
(cd example/external_client && bazelisk build //:use_graphite_dawn_capture \
    --@dawn//:dawn_enable_webgpu_on_webgpu=false)
```

## Run

```sh
# Graphite + Dawn on Metal adapter: Skia capture only
example/external_client/bazel-bin/use_graphite_dawn_capture --adapter=metal /tmp/out_metal.capt

# Graphite + Dawn WebGPU-on-WebGPU adapter: Skia capture + Dawn WebGPU capture
example/external_client/bazel-bin/use_graphite_dawn_capture --adapter=webgpu /tmp/out_webgpu.capt

# Graphite native Metal
example/external_client/bazel-bin/use_graphite_metal_capture /tmp/out_native_metal.capt
```

`--adapter` defaults to `metal`.

### Replaying existing SKPs

Pass `--skp=<file|dir>` (repeatable) to replay `.skp` files instead of the built-in demo
scene. Each SKP gets its own fresh Dawn device + Graphite context and its own capture.

```sh
# A directory of SKPs -> one capture per SKP in out/motionmark-captures/<stem>.capt*
example/external_client/bazel-bin/use_graphite_dawn_capture --adapter=webgpu \
    --skp=out/motionmark-skps out/motionmark-captures

# A single SKP -> exactly the given .capt path
example/external_client/bazel-bin/use_graphite_dawn_capture --adapter=webgpu \
    --skp=out/motionmark-skps/desk_motionmarkarcs.skp /tmp/arcs.capt
```

- If there is exactly one SKP and the output ends in `.capt`, it is used as the file name.
  Otherwise the output is treated as a directory (created if needed).
- Each SKP is drawn on a white background at its cull-rect size. Embedded PNG/JPEG/WebP
  images and typefaces (CoreText) are decoded.
- Exit code is `2` if any SKP failed or Dawn reported errors.

The Dawn example exits with code `2` if Dawn reported any uncaptured errors (they are
printed as `DAWN ERROR ...`), and `1` on setup failures (e.g. no adapter found).

> [!WARNING]
> Run the binaries directly (as above) or pass an **absolute** output path. With
> `bazelisk run`, relative paths resolve inside Bazel's runfiles sandbox, not your
> current directory.

## Output files

Files are written to the output path you pass on the command line:

| File | Written when | Contents |
|---|---|---|
| `<out>.capt` | always | Skia `SkCapture` (open with `viewer --file <out>.capt`) |
| `<out>.capt.png` | Dawn example | Readback of the final rendered surface (sanity check) |
| `<out>.capt.dawn_commands` | `--adapter=webgpu` | Dawn WebGPU capture: command stream |
| `<out>.capt.dawn_content` | `--adapter=webgpu` | Dawn WebGPU capture: content stream (buffer/texture data) |

The Skia viewer only recognizes files ending in `.capt`, so keep that extension.

## Viewing the Skia capture

With a viewer built from this Skia checkout (GN, `skia_use_dawn=true` and
`skia_enable_graphite=true` for the WebGPU Commands window):

```sh
ninja -C out/Release viewer
./out/Release/viewer --file out/motionmark-captures/desk_motionmarkarcs.capt
# or point it at a directory of .capt files
./out/Release/viewer --captures out/motionmark-captures
```

If `<capt>.dawn_commands` exists next to the `.capt`, the viewer also opens a
**WebGPU Commands** window. It lists the Dawn commands and objects, with their labels.
