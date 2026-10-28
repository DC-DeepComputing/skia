---
name: skia/avoid-duplicate-friend-declarations
description: When adding a friend declaration to a C++ class, verify friendship has not already been granted elsewhere in the class
rubric: readability
severity: nit
---

**Rule.** Do not add duplicate `friend` declarations for the same class or
function within a class definition. Check whether friendship has already been
granted elsewhere in the class before adding a new friend declaration.

**Why.** In C++, a `friend` declaration grants access to the entire class
regardless of access specifier or placement. Adding another friend declaration
for an already friended class or function is redundant, clutters the header, and
can confuse readers about class structure.

## Examples

Bad:

```cpp
class Resource {
    friend class ResourceCache;
    ...
public:
    void setLabel(std::string_view label);

    // Redundant duplicate friend declaration
    friend class ResourceCache;
};
```

Good:

```cpp
class Resource {
    friend class ResourceCache;
    ...
public:
    void setLabel(std::string_view label);
};
```
