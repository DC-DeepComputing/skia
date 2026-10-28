---
name: skia/go-mod/run-go-mod-tidy-on-dependency-updates
description: When adding, updating, or removing Go dependencies in go.mod, run go mod tidy and commit the synchronized go.sum to prune obsolete checksums
rubric: correctness
severity: should_fix
---

**Rule.** When adding, updating, or removing Go module dependencies in go.mod
(e.g., via go get or manual edits), run go mod tidy and commit the resulting
go.sum changes in the same CL. Verify that obsolete module versions and
checksums are pruned and that all indirect dependencies and checksum entries
remain fully synchronized.

**Why.** Updating dependencies in go.mod without executing go mod tidy leaves
go.sum with obsolete hashes and missing transitive checksums. Skia's housekeeper
CI bots enforce Go module cleanliness and will fail if go.sum or go.mod is not
tidied.

## Examples

Bad:

```cpp
// go.mod updated to v0.31.0, but go.sum retains obsolete versions and missing indirect checksums:
// go.sum:
golang.org/x/crypto v0.20.0 h1:...
golang.org/x/crypto v0.31.0 h1:...
golang.org/x/net v0.21.0 h1:...
golang.org/x/net v0.33.0 h1:...
```

Good:

```cpp
// Run `go mod tidy` to prune obsolete module versions and update indirect dependencies:
// go.sum:
golang.org/x/crypto v0.31.0 h1:...
golang.org/x/net v0.33.0 h1:...
```
