---
name: skia/check-skwstream-write-results
description: When writing to an SkWStream or using SkStreamPriv::Write* helpers during serialization, verify the returned boolean status and bail out on failure
rubric: correctness
severity: nit
---

**Rule.** Check the boolean return value of SkWStream write operations (such as
stream->write(), write8(), or SkStreamPriv::Write* helpers) during serialization
or encoding. Bail out immediately by returning nullptr or an error code if any
write operation returns false, or accumulate write statuses to ensure errors are
propagated.

**Why.** Stream write operations can fail due to allocation failure,
out-of-memory conditions, or stream errors. Ignoring write return values causes
serialization routines to proceed with incomplete data, emitting truncated or
corrupted byte buffers that fail during later decoding.

## Examples

Bad:

```cpp
// Write the number of tags and data without checking write status.
SkStreamPriv::WriteU16BE(&stream, numSubIFDTags);
stream.write(tagData, tagSize);
```

Good:

```cpp
// Verify write operations succeed and bail out on stream failure.
if (!SkStreamPriv::WriteU16BE(&stream, numSubIFDTags) ||
    !stream.write(tagData, tagSize)) {
    return nullptr;
}
```
