---
name: skia/fuzz/do-not-check-in-seed-corpora
description: When adding or updating fuzzers in fuzz/, do not check in seed corpora or binary inputs to git; upload them to Google Cloud Storage
rubric: readability
severity: should_fix
---

**Rule.** Do not check in fuzzing seed corpora or binary test cases into the git
repository under fuzz/. Instead, upload seed corpora to Google Cloud Storage
(gs://skia-cdn/oss-fuzz/) and configure OSS-Fuzz build scripts (Dockerfile /
build.sh) to download and package them as <fuzzer_name>_seed_corpus.zip.

**Why.** Checking binary corpus files into git permanently bloats repository
clone and fetch size. Skia's fuzzing infrastructure fetches seed corpora
directly from Google Cloud Storage during OSS-Fuzz builds, keeping the git
repository lightweight.

## Examples

Bad:

```cpp
// Checking binary seed corpora directly into the git repository:
fuzz/oss_fuzz/hdr_agtm.corpus/seed_0.bin
fuzz/oss_fuzz/hdr_agtm.corpus/seed_1.bin
```

Good:

```cpp
// Upload seed corpus archive to Google Cloud Storage and reference in OSS-Fuzz:
// 1. Upload corpus zip to gs://skia-cdn/oss-fuzz/hdr_agtm_seed_corpus.zip
// 2. Download and package via OSS-Fuzz Dockerfile and build.sh
```
