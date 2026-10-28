---
name: skia/codec/validate-multi-segment-metadata-chunks
description: When reassembling multi-segment chunked metadata in image codecs, validate segment indices, reject zero or mismatched part counts and duplicates, and ensure all parts are present
rubric: correctness
severity: should_fix
---

**Rule.** When parsing and reassembling multi-segment chunked metadata (such as
JPEG APP2 ICC profiles, ISO gainmaps, or JUMBF), validate that sequence indices
are 1-based and do not exceed the total count, verify that total part counts are
non-zero and consistent across chunks, reject duplicate parts, and verify that
all segments are present before concatenating payloads.

**Why.** Concatenating segments without validating sequence bounds, total chunk
counts, and set completeness allows malformed or corrupted metadata to be
processed, leading to out-of-bounds indexing, truncated profiles, and behavioral
divergence from standard decoders.

## Examples

Bad:

```cpp
pub fn extract_icc_profile(raw_data: &[u8], segments: &[ScannedSegment]) -> Vec<u8> {
    let mut icc_parts: Vec<(u8, Vec<u8>)> = Vec::new();
    for seg in segments {
        let seq_num = param_data[12];
        let payload = &param_data[ICC_HEADER_SIZE..];
        icc_parts.push((seq_num, payload.to_vec()));
    }
    // Concatenates whatever parts were found without validating bounds or completeness.
    icc_parts.sort_by_key(|(seq, _)| *seq);
    let mut result = Vec::new();
    for (_, data) in &icc_parts {
        result.extend_from_slice(data);
    }
    result
}
```

Good:

```cpp
pub fn extract_icc_profile(raw_data: &[u8], segments: &[ScannedSegment]) -> Vec<u8> {
    let mut parts: Vec<Option<Vec<u8>>> = Vec::new();
    let (mut found_parts, mut expected_parts) = (0usize, 0u8);
    for seg in segments {
        let (seq_num, part_count) = (param_data[12], param_data[13]);
        if part_count == 0 || seq_num == 0 || seq_num > part_count {
            return Vec::new();
        }
        if expected_parts == 0 {
            expected_parts = part_count;
            parts.resize_with(expected_parts as usize, || None);
        } else if part_count != expected_parts {
            return Vec::new();
        }
        let idx = (seq_num - 1) as usize;
        if parts[idx].is_some() { return Vec::new(); } // Reject duplicates.
        parts[idx] = Some(payload.to_vec());
        found_parts += 1;
    }
    if expected_parts == 0 || found_parts != expected_parts as usize {
        return Vec::new(); // Reject incomplete sets.
    }
    let mut result = Vec::new();
    for part in parts.into_iter().flatten() {
        result.extend(part);
    }
    result
}
```
