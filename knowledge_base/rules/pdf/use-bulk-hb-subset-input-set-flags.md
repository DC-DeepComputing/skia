---
name: skia/pdf/use-bulk-hb-subset-input-set-flags
description: When configuring HarfBuzz font subsetting options on hb_subset_input_t, use hb_subset_input_set_flags with bitwise-OR'd flags instead of calling hb_subset_input_set_flag repeatedly
rubric: readability
severity: should_fix
---

**Rule.** When configuring HarfBuzz font subsetting options on
hb_subset_input_t, use hb_subset_input_set_flags with bitwise-OR'd
hb_subset_flags_t values rather than calling hb_subset_input_set_flag repeatedly
for each individual flag.

**Why.** Using hb_subset_input_set_flags sets all subsetting options in a single
call with bitwise-OR'd flags, avoiding repeated function call overhead and
keeping subsetting option configuration concise and idiomatic.

## Examples

Bad:

```cpp
hb_subset_input_set_flag(input.get(), HB_SUBSET_FLAG_RETAIN_GIDS, true);
hb_subset_input_set_flag(input.get(), HB_SUBSET_FLAG_NO_HINTING, true);
```

Good:

```cpp
hb_subset_input_set_flags(input.get(), HB_SUBSET_FLAGS_RETAIN_GIDS | HB_SUBSET_FLAGS_NO_HINTING);
```
