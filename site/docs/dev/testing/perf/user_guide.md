---
title: "Perf User & Triage Guides"
linkTitle: "User Guide"
weight: 10
---

Perf provides several web interfaces for exploring data, triaging regressions, and querying performance benchmarks.

## Comprehensive Guides

Detailed user guides are maintained in the [Perf Buildbot repository](https://skia.googlesource.com/buildbot/+/refs/heads/main/perf/):

### 1. [Multigraph User Guide](https://skia.googlesource.com/buildbot/+/refs/heads/main/perf/multigraph-guide.md)
Learn how to use the Multigraph Page (`/m`):
* Search, filter, and select multiple performance metrics.
* Plot multiple benchmark traces side-by-side using the interactive Test Picker.
* Compare trends across different bot configurations and OS targets.

### 2. [Data Ingestion & Upload Format Guide](https://skia.googlesource.com/buildbot/+/refs/heads/main/perf/FORMAT.md)
Learn how to format and upload benchmark data to Perf:
* Format benchmark outputs into the Perf JSON schema.
* Set up Google Cloud Storage (GCS) bucket paths for automated ingestion.
* Use the `perf-tool` CLI to validate dataset schemas locally before uploading.
