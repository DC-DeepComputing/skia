`SkImage::refEncodedData()` and `SkImageGenerator::refEncodedData()` now returns a const pointer to the SkData to more explicitly signal
that this is a read-only view into the data.

