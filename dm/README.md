# Diamond Master (`dm/`)

## Overview & Scope
`dm` (Diamond Master) is Skia's primary command-line test runner and golden-image generator used across local development and Continuous Integration (CI). It executes two categories of work in a single binary:
1. **The `Src` × `Sink` Rendering Matrix**: Pairs drawing sources (GMs, SKPs, SVGs, Lottie animations, image codecs) against rendering backends/configurations (CPU raster, Ganesh GPU, Graphite GPU, PDF, SVG, SKP) to detect crashes, rendering assertions, and visual diffs (via Skia Gold).
2. **Unit & GPU Tests**: Runs all registered `skiatest::Test` suites from [`tests/`](../tests/README.md).

## Key Concepts & Entrypoints

### Core Files
* [`dm/DM.cpp`](DM.cpp): Main entrypoint (`main()`), CLI flag parsing, `Src`/`Sink`/test gathering, threadpool scheduling, crash handling, and output hashing.
* [`dm/DMSrcSink.h`](DMSrcSink.h) & [`dm/DMSrcSink.cpp`](DMSrcSink.cpp): Defines the `DM::Src`, `DM::Sink`, and `DM::Via` abstractions and their concrete implementations.
* [`dm/DMGpuTestProcs.cpp`](DMGpuTestProcs.cpp): Implements `RunWithGaneshTestContexts` and `RunWithGraphiteTestContexts` hooks required by `tests/Test.h`.
* [`dm/DMJsonWriter.h`](DMJsonWriter.h) & [`dm/DMJsonWriter.cpp`](DMJsonWriter.cpp): Writes `dm.json` metadata and MD5 image hashes consumed by Skia Gold in CI.

### The `Src` × `Sink` Matrix (`dm/DMSrcSink.h`)
Every rendering task (`DM::Task`) pairs one `TaggedSrc` with one `TaggedSink` and returns a `DM::Result` (`Ok`, `Fatal`, or `Skip`):

* **Sources (`DM::Src`)** — *What to draw* (selected via `--src`):
  * `gm` (`GMSrc`): Golden Master tests registered in [`gm/`](../gm/README.md).
  * `image` / `colorImage` (`CodecSrc`, `AndroidCodecSrc`, `ImageGenSrc`, `ColorCodecSrc`, `BRDSrc`): Image decoding tests (full, scanline, stripe, subset, scaled, animated) from `--colorImages`.
  * `skp` / `mskp` (`SKPSrc`, `MSKPSrc`, `BisectSrc`): Serialized `SkPicture` and multi-page picture playback from `--skps` / `--mskps` / `--bisect`.
  * `svg` (`SVGSrc`): SVG DOM rendering from `--svgs`.
  * `lottie` (`SkottieSrc`): Bodymovin/Lottie animation filmstrips from `--lotties`.
* **Sinks (`DM::Sink`)** — *How/where to draw* (selected via `--config`, parsed by [`tools/flags/CommonFlagsConfig.h`](../tools/flags/CommonFlagsConfig.h)):
  * **CPU Raster (`RasterSink`)**: `8888`, `565`, `4444`, `r8`, `1010102`, `f16`, `f16norm`, `f32`, `srgba` (often combined with color-space via prefixes such as `srgb-8888`, `linear-f16`, `narrow-8888`, `p3-8888`, `rec2020-f16`).
  * **Ganesh GPU (`GPUSink` & variants)**: `gl`, `gles`, `vk`, `mtl`, `d3d`, `angle_*`, plus specialized sinks like `GPUDDLSink` (`glddl`, `vkddl`, `mtlddl`), `GPUSlugSink` (`glslug`), and `GaneshPrecompileTestingSink`.
  * **Graphite GPU (`GraphiteSink` & variants)**: `grmtl`, `grvk`, `grdawn_vk`, `grdawn_mtl`, `grdawn_d3d11`, `grdawn_d3d12`, `grdawn_gl`, `grdawn_gles`, plus `GraphitePrecompileTestingSink` (`*testprecompile`).
  * **Vector / Document & Utility Sinks**: `pdf` (`PDFSink`), `svg` (`SVGSink`), `skp` (`SKPSink`), `xps` (`XPSSink`), `null` (`NullSink`).
  * **Vias (`DM::Via`)**: Decorator sinks that wrap another sink to test indirect rendering paths (`ViaPicture` / `pic-8888`, `ViaSerialization` / `serialize-8888`, `ViaMatrix`, `ViaUpright`, `ViaSVG`).

### Threading & Execution Order
`dm` maximizes throughput while keeping GPU contexts thread-safe:
1. **CPU Serial Tests**: `DEF_SERIAL_TEST` unit tests run first on the main thread while no other threads are active.
2. **Parallel Threadpool (`SkTaskGroup`)**: Thread-safe `Src` × `Sink` pairs (such as CPU raster and vector sinks) and `DEF_TEST` CPU unit tests run concurrently across worker threads (`--threads` / `-j`).
3. **Main-Thread Serial Queue**: Any `Src` or `Sink` returning `serial() == true` (including all Ganesh `GPUSink` and Graphite `GraphiteSink` tasks), followed by `DEF_GANESH_TEST` and `DEF_GRAPHITE_TEST` suites, executes serially on the main thread. PNG encoding and MD5 hashing are offloaded back to the background threadpool.

## Key CLI Flags
* `--match` / `-m`: Filter test/GM names (`^exact$`, `substr`, `~exclude`).
* `--src`: Space-separated list of source types (`tests`, `gm`, `skp`, `mskp`, `lottie`, `svg`, `image`, `colorImage`, `bisect`).
* `--config`: Space-separated list of sink configs (`8888`, `gl`, `vk`, `mtl`, `grmtl`, `grvk`, `grdawn_vk`, `grdawn_mtl`, `pdf`, `null`, etc.).
* `--skip`: Quadruples of `(config src srcOptions name)` to skip (`_` matches any, `~` negates).
* `--writePath` / `-w <dir>` & `--readPath` / `-r <dir>`: Write rendered outputs and `dm.json` to `<dir>`, or compare MD5 hashes against a golden directory.
* `--nogpu` / `--nocpu` / `--nographite`: Disable Ganesh GPU, CPU, or Graphite test execution.
* `--threads` / `-j <N>`: Set worker thread count (use `-j 0` or `-j 1` when debugging concurrency issues).

## Build Configuration
`dm` is built in GN when `skia_enable_tools = true` (defined in [`BUILD.gn`](../BUILD.gn), linking `gn/tests.gni` and `gn/gm.gni`), and in Bazel via `//dm:dm` ([`dm/BUILD.bazel`](BUILD.bazel)).
