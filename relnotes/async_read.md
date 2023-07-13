The previous `asyncRescaleAndReadPixels` and `asyncRescaleAndReadPixelsYUV420` methods on `SkSurface`,
`SkImage`, and `skgpu::graphite::Context` have been depecrated.

They are replaced by a new overload of `asyncRescaleAndReadPixels` and a new method
`asyncRescaleAndReadPixelsYUV`. The new methods use callback functions that take new result types
that receive results as `SkPixmap` or `SkYUVAPixmaps`. Moreover, the new YUV function
takes a more complete description of the YUV result as
`SkYUVAInfo` and `SkYUVAPixmapInfo::DataType`. No new planar
configurations other than YUV420 bytes are currently supported. However,
support can be expanded without additional API changes.