`SkCodec` supports decoding to YUVA planes with a scale factor. The user passes a desired scale factor
to `SkCodec::queryYUVAInfo()` and the resulting `SkYUVAPixmapsInfo`'s dimensions will be
the codec's recommended and supported dimensions at closest to that scale factor. This has been implemented
for `SkJpegCodec`.
