---
name: skia/symmetric-growth-and-shrink-container-traits
description: When implementing reserve() or capacity policies for hash tables and bounded-load containers, account for maximum load factor and query trait hooks symmetrically
rubric: correctness
severity: should_fix
---

**Rule.** When implementing reserve(n) for hash tables or bounded-load
containers, calculate the required bucket or slot capacity using the maximum
load factor (e.g., ensuring capacity >= n / max_load_factor) so inserting n
elements does not trigger an immediate resize. Additionally, when container
growth or shrinkage policies are customizable via traits, provide symmetric
ShouldGrow and ShouldShrink hooks enforced at compile time, and query them in
reserve() rather than retaining hardcoded sizing thresholds.

**Why.** Open-addressing hash tables rehash when occupancy exceeds their maximum
load factor; reserving space without accounting for this load factor causes
subsequent insertions to immediately trigger an unwanted rehash, defeating the
reservation. Furthermore, failing to query custom growth traits in reserve() or
omitting symmetric shrink traits causes premature reallocations or resize
thrashing when custom load factors are configured.

## Examples

Bad:

```cpp
// 1. reserve() ignores maximum load factor (causes immediate rehash on n insertions):
void reserve(int n) {
    int newCapacity = SkNextPow2(n);
    if (newCapacity > fCapacity) {
        this->resize(newCapacity);
    }
}

// 2. Asymmetric traits missing ShouldShrink, and reserve() hardcodes load factor:
template <typename U>
struct HasShouldGrow<U, std::void_t<decltype(U::ShouldGrow(std::declval<int>(), std::declval<int>()))>> : std::true_type {};
// Missing HasShouldShrink static_assert; reserve() hardcodes (n * 4 > newCapacity * 3)
```

Good:

```cpp
// reserve() accounts for maximum load factor and queries customizable traits:
void reserve(int n) {
    int newCapacity = SkNextPow2(n);
    bool shouldGrow = false;
    if constexpr (HasShouldGrow<Traits>::value) {
        shouldGrow = Traits::ShouldGrow(n, newCapacity);
    } else {
        shouldGrow = (n * 4 > newCapacity * 3); // Account for default 75% max load factor
    }
    if (shouldGrow) {
        newCapacity *= 2;
    }
    if (newCapacity > fCapacity) {
        this->resize(newCapacity);
    }
}

// Enforce symmetric growth/shrink traits at compile time:
template <typename U>
struct HasShouldGrow<U, std::void_t<decltype(U::ShouldGrow(std::declval<int>(), std::declval<int>()))>> : std::true_type {
    static_assert(HasShouldShrink<U>::value, "Traits defining ShouldGrow must also provide ShouldShrink().");
};
```
