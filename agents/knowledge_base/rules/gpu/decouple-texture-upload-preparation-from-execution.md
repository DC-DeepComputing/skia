---
name: skia/gpu/decouple-texture-upload-preparation-from-execution
description: When implementing GPU texture upload operations, decouple shared data preparation into an intermediate UploadSource object instead of duplicating format-specific backend virtuals
rubric: design
severity: should_fix
---

**Rule.** Decouple texture upload operations into a shared data preparation
phase (validating mip levels, dimensions, and format metadata into an
UploadSource intermediate) and an execution phase (direct host copy or GPU
transfer buffer). In Texture base classes and backend implementations, expose
unified upload execution virtual methods (such as canUploadOnHost() and
uploadDataOnHost(const UploadSource&)) rather than adding format-specific
virtual methods.

**Why.** Texture upload paths across formats share extensive validation,
mip-level calculation, and compatibility checking. Proliferating format-specific
virtual methods on backend Texture classes duplicates preparation logic across
backends and complicates introducing alternative upload pathways, whereas
encapsulating prepared data into an UploadSource standardizes validation and
unifies backend execution entry points.

## Examples

Bad:

```cpp
// Backend Texture base class exposes separate virtuals per data format,
// duplicating validation and mip calculations across backend subclasses:
virtual bool updateDataOnHost(const SkColorInfo& colorInfo, SkSpan<const MipLevel> levels);
virtual bool updateCompressedDataOnHost(const void* data, size_t dataSize);
```

Good:

```cpp
// Unified data preparation in UploadSource handles validation for all formats;
// Texture base class exposes unified upload execution virtuals:
virtual bool canUploadOnHost() const { return false; }
virtual bool uploadDataOnHost(const UploadSource& source);
```
