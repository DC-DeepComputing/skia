---
name: skia/prefer-and-then-over-map-or-none
description: When mapping a Rust Option with a closure that returns an Option, use opt.and_then(...) instead of opt.map_or(None, ...)
rubric: readability
severity: nit
---

**Rule.** When transforming a Rust Option with a closure that returns an Option,
use opt.and_then(...) instead of opt.map_or(None, ...) or
.map(...).unwrap_or(None). Reserve opt.map_or(default, ...) for non-optional
default fallback values.

**Why.** Option::and_then is Rust's canonical monadic bind/flatten operation.
Using map_or(None, ...) is verbose, redundant, and obscures the intent of
chaining fallible operations.

## Examples

Bad:

```cpp
// Verbose and unidiomatic flattening of nested Option
let format = outlines.map_or(None, |o| o.format());
```

Good:

```cpp
// Idiomatic monadic bind using and_then
let format = outlines.and_then(|o| o.format());
```
