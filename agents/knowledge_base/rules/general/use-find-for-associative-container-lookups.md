---
name: skia/use-find-for-associative-container-lookups
description: When querying associative containers like std::map or THashMap by key, use member find() or contains() instead of linear for-loop iteration
rubric: safety
severity: should_fix
---

**Rule.** When querying or looking up elements by key in associative containers
(such as std::map, std::unordered_map, or skia_private::THashMap), use the
container's lookup methods (such as find() or contains()) instead of linearly
iterating over entries with a for loop. Verify the lookup result iterator or
pointer against end() or nullptr before accessing mapped values.

**Why.** Associative containers provide logarithmic or constant-time lookups
(O(log N) or O(1)) using internal tree or hash structures. Iterating through all
key-value pairs with a linear loop to find a matching key degrades lookups to
O(N), defeating the container's performance advantages and cluttering code.

## Examples

Bad:

```cpp
// Bad: Linearly iterating over an associative container to find a key degrades to O(N)
void onTextProperty(const char node_name[],
                    const LazyHandle<skottie::TextPropertyHandle>& t) override {
    for (auto& i : fTextMap) {
        if (node_name == i.first) {
            auto value = t()->get();
            value.fText = i.second;
            t()->set(value);
        }
    }
}
```

Good:

```cpp
// Good: Using member find() provides O(1) / O(log N) key lookup
void onTextProperty(const char node_name[],
                    const LazyHandle<skottie::TextPropertyHandle>& t) override {
    const auto it = fTextMap.find(node_name);
    if (it != fTextMap.end()) {
        auto value = t()->get();
        value.fText = it->second;
        t()->set(value);
    }
}
```
