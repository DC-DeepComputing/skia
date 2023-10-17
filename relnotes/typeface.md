`SkTypeface::MakeFromName`, `SkTypeface::MakeFromFile`, `SkTypeface::MakeFromStream`, and
`SkTypeface::MakeFromData` now take an SkFontMgr. The existing versions are deprecated and will
be removed eventually.

`SkTypeface::MakeDefault()` has been deprecated. Soon it will return an empty typeface and
eventually be removed.