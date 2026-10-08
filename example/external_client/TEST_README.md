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

### Draw id tracking (`--track-draw-ids`)

Tags every GPU draw call with the Skia draw commands batched into it. Graphite inserts a WebGPU
debug marker per (Skia draw, RenderStep) before the draw call
(`sk d=<PaintersDepth> c=<asset token> i=<draw ordinal> s=<RenderStep>[ clip]`). Nested pictures
are unrolled so that each SKP command is its own capture draw.

```sh
example/external_client/bazel-bin/use_graphite_dawn_capture --adapter=webgpu --track-draw-ids \
    --skp=out/motionmark-skps out/motionmark-captures-ids
./out/Release/viewer --file out/motionmark-captures-ids/desk_motionmarkleaves.capt
```

In the viewer, click a draw in **Command History** to highlight its GPU draws in **WebGPU Commands**.
You can also expand a GPU draw call there and click one of its Skia draws to select it.

With `--track-draw-ids`, every render pass also gets its own depth attachment. Each one is
stored, can be bound as a texture, and is labeled `SkDrawIdDepth#<n>`. Graphite writes a unique
depth value for each draw, so the depth buffer tells you which draw last touched each pixel.
In the viewer's **Draw ID (depth)** window:

1. Pick a render pass. The default is the pass with the most draws.
2. Click **Replay capture**. The viewer replays the `.dawn_commands` and `.dawn_content` files on
   its own Dawn (Metal) device, then reads back that pass's color target and depth.
3. Pick what the canvas shows:
   - **Skia playback**: the SkCapture drawn by the viewer itself.
   - **GPU color**: the pass's color target from the replayed WebGPU capture.
   - **Draw ID colors**: each pixel gets a color hashed from the Skia draw that last wrote its
     depth.

   **Highlight selected skia draw** works with any of the three. It dims every pixel that the
   selected draw didn't write last. Hover a pixel to see its depth, Skia draw, GPU draw and
   RenderStep. Click a pixel in a GPU view to select that Skia draw.
4. Turn on **Step: replay only part of the capture** and pick what to step by:
   - **Skia draw** replays up to the selected Skia draw's last GPU draw. Change the selection by
     clicking in Command History or on the canvas, or use the buttons (`<<100`, `<<10`,
     `< Prev`, `Next >`, `10>>`, `100>>`). Graphite sorts and batches draws, so this isn't strictly
     cumulative: later Skia draws that were sorted earlier also appear.
   - **GPU draw (real GPU order)** replays GPU draws `0..N` in the order the GPU executed them,
     which is what you want for debugging rendering artifacts. Use the same buttons, or drag the
     slider (the replay runs on release). **Go to selected skia draw** jumps to that draw's last
     GPU draw. The window lists the Skia draws batched into GPU draw `N`; click one to select it.
     **Highlight** dims every pixel that GPU draw `N` didn't write last.

   Color and depth are both read after the replayed draws. Every command that isn't a draw still
   runs.

Step mode relies on a local Dawn patch,
[`bazel/external/dawn/replay_draw_limit.patch`](../../bazel/external/dawn/replay_draw_limit.patch).
It adds `dawn::replay::Replay::SetDrawLimit` and `Replay::GetObjectById`. The patch is applied to
`third_party/externals/dawn` for the GN viewer and listed in `bazel/cpp_modules.bzl` for Bazel.

Limitations:

- Each pixel reports only the last draw that wrote depth.
- Clip draws can make a pixel point to the wrong draw.
- `DrawListLayer` is not supported.

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
