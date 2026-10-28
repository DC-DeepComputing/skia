---
name: skia/assign-match-expressions-directly
description: When initializing variables, struct fields, or return values in Rust, evaluate match expressions directly instead of pre-declaring mutable variables or single-use bindings
rubric: readability
severity: should_fix
---

**Rule.** In Rust, evaluate `match` and `if` expressions directly at the
destination—binding directly to variables (`let var = match ...`), struct
fields, or return statements. Do not pre-declare mutable variables beforehand
and reassign them inside match arms with dummy `()` arms, and do not introduce
single-use intermediate `let` bindings that are immediately forwarded on the
next line.

**Why.** Rust is an expression-oriented language where `match` and `if` blocks
evaluate directly to values. Initializing bindings directly from match
expressions avoids unnecessary mutability (`let mut`), eliminates dummy no-op
arms, clarifies dataflow, and removes single-use temporary variables that add
visual noise.

## Examples

Bad:

```cpp
// Redundant intermediate binding:
let connection_space = match profile.pcs {
    DataColorSpace::Xyz => ColorSpace::XYZ,
    DataColorSpace::Lab => ColorSpace::Lab,
    _ => ColorSpace::Unknown,
};
out.connection_space = connection_space;

// Pre-declaring mutable variable and reassigning in arms:
let mut result = png::Transformations::IDENTITY;
match info.color_type {
    png::ColorType::Rgb => { result = png::Transformations::EXPAND; }
    png::ColorType::Indexed => (),
}
```

Good:

```cpp
// Assign directly to destination field or return:
out.connection_space = match profile.pcs {
    DataColorSpace::Xyz => ColorSpace::XYZ,
    DataColorSpace::Lab => ColorSpace::Lab,
    _ => return false,
};

// Initialize variable directly from match expression:
let result = match info.color_type {
    png::ColorType::Rgb => png::Transformations::EXPAND,
    png::ColorType::Indexed => png::Transformations::IDENTITY,
};
```
