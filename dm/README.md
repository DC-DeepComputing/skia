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
  * `image` / `colorImage` (`CodecSrc`, `AndroidCodecSrc`, `ImageGenSrc`, `ColorCodecSrc`, `BRDSrc`): Image decoding tests (full, scanline, stripe, subset, scaled, animated) from `--images`.
  * `skp` / `mskp` (`SKPSrc`, `MSKPSrc`, `BisectSrc`): Serialized `SkPicture` and multi-page picture playback from `--skps` / `--mskps`.
  * `svg` (`SVGSrc`): SVG DOM rendering from `--svgs`.
  * `lottie` (`SkottieSrc`): Bodymovin/Lottie animation filmstrips from `--lotties`.
* **Sinks (`DM::Sink`)** — *How/where to draw* (selected via `--config`, parsed by [`tools/flags/CommonFlagsConfig.h`](../tools/flags/CommonFlagsConfig.h)):
  * **CPU Raster (`RasterSink`)**: `8888`, `565`, `f16`, `srgb`, `esrgb`, `narrow`, `p3`, `rec2020`.
  * **Ganesh GPU (`GPUSink` & variants)**: `gl`, `gles`, `vk`, `mtl`, `d3d`, `angle_*`, plus specialized sinks like `GPUDDLSink` (`ddl`), `GPUSlugSink` (`slug`), and `GaneshPrecompileTestingSink`.
  * **Graphite GPU (`GraphiteSink` & variants)**: `grmtl`, `grvk`, `grdawn`, `grdawn_vk`, `grdawn_mtl`, `grdawn_d3d11`, `grdawn_d3d12`, `grdawn_gl`, plus `GraphitePrecompileTestingSink` (`*testprecompile`).
  * **Vector / Document & Utility Sinks**: `pdf` (`PDFSink`), `svg` (`SVGSink`), `skp` (`SKPSink`), `xps` (`XPSSink`), `null` (`NullSink`).
  * **Vias (`DM::Via`)**: Decorator sinks that wrap another sink to test indirect rendering paths (`ViaPicture` / `pic-8888`, `ViaSerialization` / `serialize-8888`, `ViaMatrix`, `ViaUpright`, `ViaSVG`).

### Threading & Execution Order
`dm` maximizes throughput while keeping GPU contexts thread-safe:
1. **CPU Serial Tests**: `DEF_SERIAL_TEST` unit tests run first on the main thread while no other threads are active.
2. **Parallel Threadpool (`SkTaskGroup`)**: Thread-safe `Src` × `Sink` pairs (such as CPU raster and vector sinks) and `DEF_TEST` CPU unit tests run concurrently across worker threads (`--threads` / `-j`).
3. **Main-Thread Serial Queue**: Any `Src` or `Sink` returning `serial() == true` (including all Ganesh `GPUSink` and Graphite `GraphiteSink` tasks), followed by `DEF_GANESH_TEST` and `DEF_GRAPHITE_TEST` suites, executes serially on the main thread. PNG encoding and MD5 hashing are offloaded back to the background threadpool.

## Common CLI Flags & Workflows

```sh
# List all available sources and sinks matching a filter without running them
out/Debug/dm --list --match blur

# Run unit tests and GMs matching "path" on CPU (8888) and Graphite Metal (grmtl)
out/Debug/dm --src tests gm --config 8888 grmtl --match path

# Run only GMs, skip GPU tests, and write output PNGs + dm.json to /tmp/dm_out
out/Debug/dm --src gm --nogpu --config 8888 pdf -w /tmp/dm_out

# Skip specific (config src srcOptions name) quadruples ('_' matches any, '~' negates)
out/Debug/dm --skip grmtl gm _ my_flaky_gm
```

* `--match` / `-m`: Filter test/GM names (`^exact$`, `substr`, `~exclude`).
* `--src`: Space-separated list of source types (`tests`, `gm`, `skp`, `mskp`, `lottie`, `svg`, `image`, `colorImage`).
* `--config`: Space-separated list of sink configs (`8888`, `gl`, `vk`, `mtl`, `grmtl`, `grvk`, `grdawn`, `pdf`, `null`, etc.).
* `--writePath` / `-w <dir>`: Write rendered `.png`/`.pdf` outputs and `dm.json` to `<dir>`.
* `--readPath` / `-r <dir>`: Compare rendered MD5 hashes against a previous golden directory.
* `--nogpu` / `--nocpu` / `--nographite`: Disable GPU (Ganesh), CPU, or Graphite test execution.
* `--threads` / `-j <N>`: Set worker thread count (use `-j 0` or `-j 1` when debugging concurrency issues).
