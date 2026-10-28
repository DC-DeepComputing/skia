---
name: skia/avoid-single-caller-helper-indirection
description: When implementing single-caller internal helpers, constructors, or methods requiring class state, inline logic directly instead of introducing single-use member helpers, intermediate tuple wrappers, or freestanding forwarding shims
rubric: design
severity: should_fix
---

**Rule.** Do not introduce single-caller internal helpers, member methods, or
factory functions that add unnecessary indirection, intermediate return
structures (such as packing and unpacking a std::tuple or std::pair), or
artificial abstraction boundaries. When helper logic is only used once within a
single caller or constructor, inline the initialization or operation directly
into that caller rather than defining a separate helper method or wrapper.
Similarly, when a freestanding anonymous-namespace helper requires class
instance state and merely backs a trivial one-line forwarding member function,
implement the logic directly inside the member function rather than plumbing
state through a freestanding shim.

**Why.** Single-use internal helpers, member methods, and freestanding shims
clutter interfaces, add unnecessary call indirection, and introduce boilerplate
parameter plumbing or packaging (such as tuples or structured bindings) without
genuine reuse. Inlining single-caller logic directly into constructors, calling
functions, or owning member methods keeps execution flow transparent, simplifies
debugging, and minimizes code indirection.

## Examples

Bad:

```cpp
// 1. Intermediate tuple wrapper with single caller:
static std::tuple<SkCodec::Result, std::unique_ptr<SkCodec>> Make(
        std::unique_ptr<SkStream> stream,
        std::unique_ptr<SkCodecs::ColorProfile> defaultColorProfile);
std::unique_ptr<SkCodec> SkJpegCodec::MakeFromStream(...) {
    auto [r, codec] = SkJpegCodec::Make(std::move(stream), std::move(defaultColorProfile));
    *result = r;
    return std::move(codec);
}

// 2. Single-use helper member method called only in constructor:
SkColorConverter::SkColorConverter(const SkColor* colors, int count) {
    (void)this->convert({colors, (size_t)count});
}

// 3. Freestanding helper taking class state for a trivial forwarding member:
static std::string to_mangled_wgsl_type(const Type& type, const ProgramSettings& settings);
std::string WGSLCodeGenerator::getMangledTypeName(const Type& t) {
    return to_mangled_wgsl_type(t, fProgram.fConfig->fSettings);
}
```

Good:

```cpp
// 1. Logic merged directly into the single caller:
std::unique_ptr<SkCodec> SkJpegCodec::MakeFromStream(...) {
    auto [r, decoderMgr] = read_header(stream.get(), SaveMarkers::kYes);
    if (r != SkCodec::kSuccess) {
        *result = r;
        return nullptr;
    }
    *result = SkCodec::kSuccess;
    return std::unique_ptr<SkJpegCodec>(
            new SkJpegCodec(std::move(info), std::move(stream), decoderMgr.release(), orientation));
}

// 2. Initialization inlined directly in constructor:
SkColorConverter::SkColorConverter(const SkColor* colors, int count) {
    fColors4f.resize(count);
    std::transform(colors, colors + count, fColors4f.data(), [](SkColor c) {
        return SkColor4f::FromColor(c);
    });
}

// 3. Logic implemented directly in member method accessing instance state:
std::string WGSLCodeGenerator::getMangledTypeName(const Type& type) {
    const ProgramSettings& settings = fProgram.fConfig->fSettings;
    return std::string(to_scalar_type(type, settings));
}
```
