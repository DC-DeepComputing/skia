---
name: skia/avoid-find-before-unique-container-insert
description: When inserting elements into unique associative containers like std::set or std::unordered_set, call emplace() or insert() directly without pre-checking membership via find()
rubric: readability
severity: should_fix
---

**Rule.** Do not explicitly check container membership (such as via find() ==
end() or contains()) before calling emplace() or insert() on a unique
associative container like std::set, std::unordered_set, or std::map. Call
emplace() or insert() directly without a preceding conditional guard.

**Why.** Unique associative containers already verify uniqueness upon insertion:
emplace() and insert() safely no-op if the key is already present. Performing a
separate find() query beforehand incurs a redundant lookup and adds unnecessary
branching boilerplate.

## Examples

Bad:

```cpp
if (fUnresolvedCodepoints.find(unichar) == fUnresolvedCodepoints.end()) {
    fUnresolvedCodepoints.emplace(unichar);
}
```

Good:

```cpp
fUnresolvedCodepoints.emplace(unichar);
```
