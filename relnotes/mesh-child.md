`SkMesh::Make` and `SkMesh::MakeIndexed` now accept a list of child effects via a
`SkSpan<ChildPtr>`, much like Runtime Effects. This functionality is still a work in progress;
passing child effects is not yet supported and will cause `Make` to return an error.
