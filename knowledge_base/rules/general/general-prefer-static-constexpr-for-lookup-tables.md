---
name: skia/general-prefer-static-constexpr-for-lookup-tables
description: When defining fixed lookup tables, dispatch arrays, or constant mappings, declare them as (static) constexpr rather than plain const, mutable static arrays, or dynamic containers
rubric: readability
severity: should_fix
---

**Rule.** When defining fixed lookup tables, function pointer dispatch tables,
character mappings, or constant numerical/matrix arrays, declare them as
constexpr (or static constexpr / std::to_array for local helpers) rather than
mutable static arrays or plain const. For fixed character tables or strings, use
static constexpr char[] (or std::to_array<char>) or static constexpr const
char*[] instead of local non-static containers like std::string[] or
std::vector<std::string>.

**Why.** Declaring compile-time constant arrays and dispatch tables as (static)
constexpr guarantees compile-time initialization, places them in read-only
memory (.rodata) rather than mutable data segments (.data), prevents accidental
modification, and avoids repeated allocation or dynamic initialization overhead
on every function invocation.

## Examples

Bad:

```cpp
// Bad: mutable static array of function pointers allocated in .data
static Proc gBWProcs[] = {
    bw_pt_hair_proc, bw_line_hair_proc, bw_poly_hair_proc
};

// Bad: plain const on compile-time numerical lookup table
const float JPEG_full_rgb_to_yuv[] = {
    0.299000f, 0.587000f, 0.114000f, 0.000000f, 0.000000f,
};

// Bad: local std::string array allocates and destroys strings per call
const std::string shades[] = {"0", "1", "2", "3", "4", "5", "6", "7"};
```

Good:

```cpp
// Good: static constexpr table of function pointers in read-only memory
static constexpr auto gBWProcs = std::to_array<Proc>({
    bw_pt_hair_proc, bw_line_hair_proc, bw_poly_hair_proc
});

// Good: constexpr ensures compile-time evaluation and zero runtime initialization
constexpr float JPEG_full_rgb_to_yuv[] = {
    0.299000f, 0.587000f, 0.114000f, 0.000000f, 0.000000f,
};

// Good: static constexpr char array evaluated at compile time with zero allocation
static constexpr char shades[] = {'0', '1', '2', '3', '4', '5', '6', '7'};
```
