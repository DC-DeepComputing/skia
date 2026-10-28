---
name: skia/core/share-path-traversal-via-skpathiter
description: When implementing path traversal or inspection across SkPath and SkPathBuilder, extract shared helpers accepting SkPathIter instead of duplicating verb loops
rubric: design
severity: should_fix
---

**Rule.** When implementing path traversal, formatting, inspection, or
serialization logic that applies to both SkPath and SkPathBuilder (or
SkPathRaw), factor the verb dispatch loop into a shared helper function
accepting an SkPathIter. Callers should pass this->iter() or path.iter() rather
than duplicating verb-iteration loops and switch statements across path classes.

**Why.** Both SkPath and SkPathBuilder provide an iter() method returning
SkPathIter. Centralizing verb iteration and handling in shared helpers operating
on SkPathIter prevents boilerplate duplication, avoids logic drift between path
representations, and ensures consistent geometry traversal behavior across path
containers.

## Examples

Bad:

```cpp
// In SkPath::dump:
SkPath::Iter iter(*this, false);
while (auto rec = iter.next()) {
    switch (rec->fVerb) { /* format verbs */ }
}

// In SkPathBuilder::dumpToString:
for (auto iter = this->iter(); auto rec = iter.next(); ) {
    switch (rec->fVerb) { /* duplicate formatting logic */ }
}
```

Good:

```cpp
static void dump_iter(SkPathIter iter, SkString* builder, const char cmdPrefix[]) {
    while (auto rec = iter.next()) {
        switch (rec->fVerb) { /* format verbs */ }
    }
}

void SkPath::dump(...) const {
    dump_iter(this->iter(), &builder, "path");
}

SkString SkPathBuilder::dumpToString(...) const {
    dump_iter(this->iter(), &builder, "");
}
```
