---
name: skia/codec/verify-destination-index-in-shared-parameter-copy
description: When copying shared or default parameters from a base array element to secondary indices, verify all destination fields target the secondary index rather than self-assigning
rubric: correctness
severity: should_fix
---

**Rule.** When replicating shared or default parameters from a base element
(such as index 0) to subsequent array elements, ensure each assignment writes to
the destination index rather than self-assigning to the base index. Audit
copy-pasted assignment blocks to ensure no array subscripts inadvertently retain
the source index.

**Why.** Copy-pasting propagation statements from earlier fields easily leads to
accidental self-assignments (e.g., arr[0] = arr[0]). Because self-assignments
compile without warning and target fields frequently default to zero in unit
tests, such bugs silently leave secondary array elements uninitialized or
unpopulated.

## Examples

Bad:

```cpp
// Bug: copy-paste error accidentally self-assigns to index 0 instead of index a.
gain_curve_num_control_points_minus_1[a] = gain_curve_num_control_points_minus_1[0];
gain_curve_use_pchip_slope_flag[0] = gain_curve_use_pchip_slope_flag[0];
```

Good:

```cpp
gain_curve_num_control_points_minus_1[a] = gain_curve_num_control_points_minus_1[0];
gain_curve_use_pchip_slope_flag[a] = gain_curve_use_pchip_slope_flag[0];
```
