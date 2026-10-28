---
name: skia/dont-evaluate-find-iterator-as-boolean
description: When checking element presence, prefer std::any_of for predicates over std::find_if, and explicitly compare std::find iterators against end() rather than evaluating them as booleans
rubric: readability
severity: should_fix
---

**Rule.** When testing whether container or span elements satisfy a predicate,
prefer standard quantifier algorithms like std::any_of, std::none_of, or
std::all_of over std::find_if(...) != end() or == end(). Never evaluate the
iterator returned by std::find or std::find_if directly as a boolean condition;
for value searches with std::find, explicitly compare the returned iterator
against end() (e.g. it != container.end()).

**Why.** Iterators for contiguous containers and spans (such as SkSpan or
std::span) are raw pointers that implicitly convert to true in boolean contexts
even when pointing to past-the-end, causing uncompared find checks in assertions
or conditionals to pass silently. For predicate searches, quantifier algorithms
directly return booleans and express intent without the boilerplate and
error-prone iterator comparisons of find_if.

## Examples

Bad:

```cpp
// Bug: raw pointer end iterator converts to true, silently passing
SkASSERT(condition || std::find_if(span.begin(), span.end(), [&](const auto& item) {
    return item.get() == target;
}));
if (std::find(items.begin(), items.end(), target)) {
    handle_found();
}

// Verbose: find_if compared against end() to test a predicate
bool exists = std::find_if(span.begin(), span.end(), pred) != span.end();
```

Good:

```cpp
// Idiomatic: use std::any_of directly for predicate tests
SkASSERT(condition || std::any_of(span.begin(), span.end(), [&](const auto& item) {
    return item.get() == target;
}));
bool exists = std::any_of(span.begin(), span.end(), pred);

// Explicit comparison against end() when searching for values
if (std::find(items.begin(), items.end(), target) != items.end()) {
    handle_found();
}
```
