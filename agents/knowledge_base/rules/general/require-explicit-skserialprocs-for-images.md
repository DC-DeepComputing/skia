---
name: skia/require-explicit-skserialprocs-for-images
description: When serializing payloads containing SkImage resources, supply explicit SkSerialProcs with fImageProc rather than relying on default procs or registered encoders
rubric: correctness
severity: should_fix
---

**Rule.** Do not rely on implicit, default, or globally registered image
encoders when serializing payloads that contain SkImage objects (such as via
SkWriteBuffer, SkBinaryWriteBuffer, or SkPicture::serialize). Unlike
deserialization where decoders can be registered globally, image serialization
requires explicit SkSerialProcs with an fImageProc callback, or image
serialization will produce null data and trigger assertion failures.

**Why.** Skia supports globally registered decoders for deserialization via
SkCodecs::Register, but provides no global registry or default encoders for
serialization. Without an explicit fImageProc in SkSerialProcs, SkWriteBuffer
cannot encode SkImage instances, causing serialize_image to fail and resulting
in dropped image data or debug assertions.

## Examples

Bad:

```cpp
// Serializing an object containing images with empty or default procs
SkBinaryWriteBuffer buffer({});
this->doFlatten(buffer);
return buffer.snapshotAsData();
```

Good:

```cpp
// Explicitly configure image serialization procs
SkSerialProcs procs;
procs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
    return SkPngEncoder::Encode(nullptr, img, SkPngEncoder::Options{});
};
SkBinaryWriteBuffer buffer(procs);
this->doFlatten(buffer);
return buffer.snapshotAsData();
```
