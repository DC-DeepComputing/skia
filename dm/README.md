# Diamond Master (`dm/`)

## Overview

`dm` (Diamond Master) is Skia's primary command-line test runner and golden-image generator used across local development and Continuous Integration (CI).

* **`Src` × `Sink` Rendering Matrix**: Pairs drawing sources (`GMSrc`, `CodecSrc`, `SKPSrc`, `SVGSrc`, `SkottieSrc`) against rendering backends (`RasterSink`, Ganesh `GPUSink`, Graphite `GraphiteSink`, `PDFSink`, `SVGSink`, `SKPSink`, and `Via` decorators) to detect crashes, rendering assertions, and visual diffs via Skia Gold.
* **Unit & GPU Test Execution**: Runs all registered `skiatest::Test` suites from [`../tests/`](../tests/).
* **Hybrid Threading Model**: Runs `DEF_SERIAL_TEST` suites first on the main thread, schedules thread-safe `Src` × `Sink` pairs and `DEF_TEST` CPU unit tests across an `SkTaskGroup` worker pool, and executes GPU sinks and `DEF_GANESH_TEST` / `DEF_GRAPHITE_TEST` suites serially on the main thread while offloading PNG encoding and MD5 hashing back to worker threads.

## Key Classes and Entrypoints

The primary classes and entrypoints in `dm/` are grouped below by concept:

| Concept | Key Files | Description |
| :--- | :--- | :--- |
| **Runner & Scheduling** | [`DM.cpp`](DM.cpp) | Main entrypoint (`main()`), CLI flag parsing, `Src`/`Sink`/test gathering, threadpool scheduling (`Task`), crash handling, and output verification. |
| **Sources, Sinks & Vias** | [`DMSrcSink.h`](DMSrcSink.h), [`DMSrcSink.cpp`](DMSrcSink.cpp) | Abstract interfaces (`DM::Src`, `DM::Sink`, `DM::Via`) and concrete implementations for GMs, codecs, SKPs, CPU raster, Ganesh, Graphite, vector backends, and indirect rendering wrappers (`ViaPicture`, `ViaSerialization`). |
| **GPU Test Context Hooks** | [`DMGpuTestProcs.cpp`](DMGpuTestProcs.cpp) | Implements `skiatest::RunWithGaneshTestContexts` and `skiatest::graphite::RunWithGraphiteTestContexts` to instantiate backend contexts for GPU unit tests. |
| **Gold Metadata & Hashing** | [`DMJsonWriter.h`](DMJsonWriter.h), [`DMJsonWriter.cpp`](DMJsonWriter.cpp) | Thread-safe accumulator (`DM::JsonWriter`) that writes `dm.json` test results and MD5 image hashes consumed by Skia Gold in CI. |

## Common Workflows

* `--match` / `-m`: Filter test or GM names (`^exact$`, `substr`, `~exclude`).
* `--src`: Space-separated list of source types (`tests`, `gm`, `skp`, `mskp`, `lottie`, `svg`, `image`, `colorImage`, `bisect`).
* `--config`: Space-separated list of sink configs (`8888`, `gl`, `vk`, `mtl`, `grmtl`, `grvk`, `grdawn_vk`, `grdawn_mtl`, `pdf`, `null`, etc.).
* `--skip`: Quadruples of `(config src srcOptions name)` to skip (`_` matches any, `~` negates).
* `--writePath` / `-w <dir>` & `--readPath` / `-r <dir>`: Write rendered outputs and `dm.json` to `<dir>`, or compare MD5 hashes against a golden directory.
* `--nogpu` / `--nocpu` / `--nographite`: Disable Ganesh GPU, CPU, or Graphite test execution.
* `--threads` / `-j <N>`: Set worker thread count (use `-j 0` or `-j 1` when debugging concurrency issues).

## Related Directories

| Directory | Description |
| :--- | :--- |
| [`../gm/`](../gm/) | Golden Master visual rendering tests executed via `GMSrc`. |
| [`../tests/`](../tests/) | Unit and GPU test suites executed by `dm`. |
| [`../tools/`](../tools/) | Shared CLI flag parsing (`flags/`), GPU test contexts (`gpu/`, `ganesh/`, `graphite/`), and resource helpers. |

