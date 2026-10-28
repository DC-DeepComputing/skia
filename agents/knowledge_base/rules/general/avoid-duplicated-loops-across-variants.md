---
name: skia/avoid-duplicated-loops-across-variants
description: When iterating over multiple variants or enum combinations, use nested loops over collections instead of duplicating loop bodies with offset indexing
rubric: design
severity: should_fix
---

**Rule.** Avoid duplicating loop bodies to iterate across multiple variants,
configurations, or enum values. Store the variant sets in collections (such as
`constexpr std::array` or `SkSpan`) and iterate over them using nested range-for
loops with a shared body, advancing target indices linearly rather than
computing manual base offsets.

**Why.** Duplicating loop bodies across combinations of parameters or enum
variants introduces redundant logic, invites code drift, and leads to fragile
index math (such as `offset + i`). Encapsulating orthogonal variant sets in
collections and iterating with nested loops makes the Cartesian product
explicit, minimizes duplication, and simplifies maintenance.

## Examples

Bad:

```cpp
for (int i = 0; i < namesCount; ++i) {
    nameRecords[i].encodingID.windows.value = SkOTTableName::Record::EncodingID::Windows::Symbol;
    nameRecords[i].nameID.predefined.value = namesToCreate[i];
    // ...
}
for (int i = 0; i < namesCount; ++i) {
    nameRecords[namesCount + i].encodingID.windows.value = SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2;
    nameRecords[namesCount + i].nameID.predefined.value = namesToCreate[i];
    // ...
}
```

Good:

```cpp
static constexpr std::array<SkOTTableName::Record::EncodingID::Windows::Value, 2> encodings{{
    SkOTTableName::Record::EncodingID::Windows::Symbol,
    SkOTTableName::Record::EncodingID::Windows::UnicodeBMPUCS2,
}};

int nrIdx = 0;
for (const auto& encoding : encodings) {
    for (const auto& name : names) {
        nameRecords[nrIdx].encodingID.windows.value = encoding;
        nameRecords[nrIdx].nameID.predefined.value = name;
        // ...
        ++nrIdx;
    }
}
```
