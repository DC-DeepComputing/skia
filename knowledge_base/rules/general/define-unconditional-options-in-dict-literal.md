---
name: skia/define-unconditional-options-in-dict-literal
description: When defining configuration options or build arguments in Python dictionaries, initialize unconditional defaults in the primary dictionary literal rather than as loose assignments
rubric: readability
severity: should_fix
---

**Rule.** In Python configuration and build generator scripts, initialize
unconditional configuration options and default arguments directly inside the
primary dictionary literal. Avoid setting unconditional keys via loose
dictionary subscript assignments (`d['option'] = value`) placed outside the
literal or interspersed among conditional override blocks.

**Why.** Defining unconditional default options directly inside the initial
dictionary literal keeps baseline configuration centralized and easy to audit.
It cleanly separates baseline defaults from subsequent conditional branches that
override settings per target platform or environment, preventing scattered state
mutations.

## Examples

Bad:

```cpp
# gn/gn_to_bp.py
d = {
    'is_official_build': 'true',
    'skia_use_perfetto': 'false',
}
...
if target_os == '"win"':
    d['skia_use_xps'] = 'false'

d['skia_use_libheif'] = 'false'
if target_os == '"android"':
    d['skia_use_vulkan'] = 'true'
```

Good:

```cpp
# gn/gn_to_bp.py
d = {
    'is_official_build': 'true',
    'skia_use_perfetto': 'false',
    'skia_use_libheif':  'false',
}
...
if target_os == '"win"':
    d['skia_use_xps'] = 'false'

if target_os == '"android"':
    d['skia_use_vulkan'] = 'true'
```
