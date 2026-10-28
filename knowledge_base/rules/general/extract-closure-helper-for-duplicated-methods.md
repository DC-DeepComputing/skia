---
name: skia/extract-closure-helper-for-duplicated-methods
description: When multiple methods share identical state validation, error mapping, and post-processing, extract a helper parameterized by a closure rather than duplicating method boilerplate
rubric: design
severity: should_fix
---

**Rule.** When multiple methods share identical boilerplate for state checks,
error mapping, and post-processing but differ only in which underlying method
they invoke, extract a helper parameterized by a closure (`impl FnOnce(...)`).
Pass a lightweight closure from each public entrypoint to the shared helper
instead of duplicating the entire method body.

**Why.** Duplicating validation, error mapping, and post-processing across
similar methods introduces maintenance hazards, increases binary size, and
invites bugs if error handling or state transitions diverge. Parameterizing the
shared execution flow with a closure consolidates invariant checks and error
translation in a single location.

## Examples

Bad:

```cpp
pub fn read_metadata(&mut self) -> DecodingResult {
    if self.metadata_loaded { return DecodingResult::Success; }
    let decoder = match &mut self.decoder {
        Some(d) => d,
        None => return DecodingResult::FormatError,
    };
    match decoder.read_metadata() {
        Ok(()) => {}
        Err(e) => return match BmpError::from(e) {
            BmpError::InsufficientData => DecodingResult::IncompleteInput,
            BmpError::UnsupportedFeature => DecodingResult::UnsupportedFeature,
            _ => DecodingResult::FormatError,
        },
    }
    self.extract_metadata_from_decoder();
    DecodingResult::Success
}

pub fn read_metadata_for_ico(&mut self) -> DecodingResult {
    if self.metadata_loaded { return DecodingResult::Success; }
    let decoder = match &mut self.decoder {
        Some(d) => d,
        None => return DecodingResult::FormatError,
    };
    match decoder.read_metadata_in_ico_format() {
        Ok(()) => {}
        Err(e) => return match BmpError::from(e) {
            BmpError::InsufficientData => DecodingResult::IncompleteInput,
            BmpError::UnsupportedFeature => DecodingResult::UnsupportedFeature,
            _ => DecodingResult::FormatError,
        },
    }
    self.extract_metadata_from_decoder();
    DecodingResult::Success
}
```

Good:

```cpp
pub fn read_metadata(&mut self) -> DecodingResult {
    self.read_metadata_impl(|d| d.read_metadata())
}

pub fn read_metadata_for_ico(&mut self) -> DecodingResult {
    self.read_metadata_impl(|d| d.read_metadata_in_ico_format())
}

fn read_metadata_impl(
    &mut self,
    read_fn: impl FnOnce(&mut BmpDecoder<DecoderInput>) -> Result<(), image::ImageError>,
) -> DecodingResult {
    if self.metadata_loaded { return DecodingResult::Success; }
    let decoder = match &mut self.decoder {
        Some(d) => d,
        None => return DecodingResult::FormatError,
    };
    match read_fn(decoder) {
        Ok(()) => {}
        Err(e) => return match BmpError::from(e) {
            BmpError::InsufficientData => DecodingResult::IncompleteInput,
            BmpError::UnsupportedFeature => DecodingResult::UnsupportedFeature,
            _ => DecodingResult::FormatError,
        },
    }
    self.extract_metadata_from_decoder();
    DecodingResult::Success
}
```
