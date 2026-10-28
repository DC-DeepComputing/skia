---
name: skia/utils/zero-initialize-scratch-buffers-in-parsing-loops
description: When staging parsed tokens into scratch buffers in loops with centralized error handling, zero-initialize buffers at declaration rather than checking errors across every branch
rubric: design
severity: should_fix
---

**Rule.** When staging parsed tokens or coordinates into stack-allocated scratch
buffers inside a loop where errors are validated centrally at the loop boundary,
zero-initialize scratch arrays at declaration with {}. Avoid scattering
redundant early-exit checks across every individual parse branch, allowing
transient operations to proceed safely with zeroed defaults without triggering
uninitialized memory reads or MemorySanitizer (MSan) warnings before the central
bailout executes.

**Why.** Scattering repetitive early-return error checks across every parsing
case clutters control flow when an outer loop or transaction boundary already
validates progress. Pre-zeroing scratch buffers ensures that failed reads do not
expose indeterminate stack garbage to downstream staging calls, satisfying MSan
while keeping error handling consolidated.

## Examples

Bad:

```cpp
// Bad: uninitialized scratch buffer requires scattering repetitive checks across cases
SkPoint points[3];
while (data[0]) {
    switch (data[0]) {
        case 'M':
            data = find_points(data, points, 1, relative, &c);
            if (!data) return false;  // Repetitive check across all cases
            builder.moveTo(points[0]);
            break;
        case 'L':
            data = find_points(data, points, 1, relative, &c);
            if (!data) return false;
            builder.lineTo(points[0]);
            break;
    }
}
```

Good:

```cpp
// Good: value-initialize scratch buffer to satisfy MSan and check errors centrally
SkPoint points[3] = {};
while (data[0]) {
    if (!data) return false;  // Handled centrally at loop boundary
    switch (data[0]) {
        case 'M':
            data = find_points(data, points, 1, relative, &c);
            builder.moveTo(points[0]);
            break;
        case 'L':
            data = find_points(data, points, 1, relative, &c);
            builder.lineTo(points[0]);
            break;
    }
}
```
