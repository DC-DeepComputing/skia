---
name: skia/document-gcs-mirror-commands-in-commit-message
description: When adding or updating external dependencies mirrored to Google Cloud Storage via gcs_mirror, document the exact mirroring command invocations in the commit message
rubric: readability
severity: should_fix
---

**Rule.** When adding or updating external dependencies or toolchain assets that
are mirrored to Google Cloud Storage (such as crates or archives mirrored via
`bazel/gcs_mirror/gcs_mirror.go`), document the exact mirroring command
invocations in the commit message body. Include all command-line arguments,
source URLs, sha256 hashes, and flags (such as `--add_suffix`) so the mirroring
process is reproducible.

**Why.** Recording the exact `gcs_mirror` command-line invocations in the commit
message provides a verifiable audit trail for mirrored binaries and serves as an
immediate, copy-pasteable reference for future dependency updates or
troubleshooting mirror issues.

## Examples

Bad:

```cpp
This CL updates the `WORKSPACE.bazel` dependency on `cxx` and `cxxbridge-cmd` crates to version `1.0.128`.
```

Good:

```cpp
This CL updates the `WORKSPACE.bazel` dependency on `cxx` and `cxxbridge-cmd` crates to version `1.0.128`.

The following commands have been used to mirror the crates:
$ go run ./bazel/gcs_mirror/gcs_mirror.go --url https://crates.io/api/v1/crates/cxxbridge-cmd/1.0.128/download --sha256 1e3d98437a33d422ba30975d549f754d50300404096f17f5037abce2d91e3cfa --add_suffix=.tar.gz
$ go run ./bazel/gcs_mirror/gcs_mirror.go --url https://crates.io/api/v1/crates/cxx/1.0.128/download --sha256 54ccead7d199d584d139148b04b4a368d1ec7556a1d9ea2548febb1b9d49f9a4 --add_suffix=.tar.gz
```
