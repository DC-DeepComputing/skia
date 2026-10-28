---
name: skia/pdf/describe-pdf-image-direct-embedding
description: When documenting image pre-encoding or compression in SkPDF, describe direct stream embedding concisely rather than writing wordy comments about generic fast paths or downstream devices
rubric: readability
severity: nit
---

**Rule.** When documenting image pre-encoding, compression, or deferred image
creation in SkPDF, explain concisely that pre-compression allows compressed data
(such as JPEG streams) to be emitted or embedded directly into the PDF without
re-encoding. Do not vaguely describe pre-encoding as an execution 'fast path',
and avoid wordy multi-line comments restating self-evident downstream
consumption (such as embedding in devices below); keep inline conversion
comments concise and focused on the emitted format.

**Why.** In PDF generation, pre-compressing an image into formats like JPEG
enables SkPDF to embed the raw compressed stream directly via DCTDecode filters,
avoiding lossy re-encoding and uncompressed raster embedding while keeping file
size small. Calling this mechanism a generic execution 'fast path' or adding
wordy comments that explain obvious downstream device use clutters the code
without clarifying the emitted stream format.

## Examples

Bad:

```cpp
// Vaguely describing pre-encoding as an execution fast path:
// By encoding this into jpeg, it can follow the fast path during drawImage.
if (SkJpegEncoder::Encode(&buffer, pm, jpegOptions)) {
    return SkCodecs::DeferredImage(SkJpegDecoder::Decoder(buffer.detachAsData()));
}

// Wordy multi-line comment restating obvious downstream device embedding:
// Convert Gray image to jpeg if needed so it can be more efficiently embedded in
// the glyphDevice below.
```

Good:

```cpp
// Stating direct stream embedding:
// By encoding this into jpeg, it can be embedded directly as a JPEG stream during drawImage.
if (SkJpegEncoder::Encode(&buffer, pm, jpegOptions)) {
    return SkCodecs::DeferredImage(SkJpegDecoder::Decoder(buffer.detachAsData()));
}

// Concise single-line comment focused on the emitted format:
// Convert Grey image to deferred jpeg image to emit as jpeg
```
