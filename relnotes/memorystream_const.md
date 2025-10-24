`SkMemoryStream` now takes in a `const SkData`, as it's a read-only view into that data.
`SkMemory::getData()` has been deprecated for `SkMemory::getDataView()` which returns a
`const sk_sp<SkData>`.