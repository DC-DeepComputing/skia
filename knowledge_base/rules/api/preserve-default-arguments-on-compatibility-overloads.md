---
name: skia/api/preserve-default-arguments-on-compatibility-overloads
description: When adding compatibility or migration function overloads in headers, replicate all default parameter values from the canonical signature to avoid breaking callers omitting those arguments
rubric: correctness
severity: should_fix
---

**Rule.** When declaring compatibility, migration, or forwarding overloads that
mirror an existing function in a header, replicate any default parameter values
(such as = nullptr, = {}, = 0, or = false) present on the canonical signature
onto the overload's corresponding trailing parameters. Because C++ does not
inherit default arguments across distinct overloads in an overload set, omitting
them causes callers that omit those defaulted arguments to fail overload
resolution or fail compilation.

**Why.** In C++, default parameter values belong to individual function
declarations and are not inherited or shared across overloads in an overload
set. Omitting default arguments on a transitional or migration wrapper breaks
backward compatibility for callers that rely on omitting those trailing
arguments when invoking the overload.

## Examples

Bad:

```cpp
// Canonical function in header has default argument '= nullptr':
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<const SkData> data,
                                       SkCodec::Result* result,
                                       SkCodecs::DecodeContext ctx = nullptr);

// Compatibility overload omits '= nullptr', breaking callers passing 2 arguments:
inline std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                       SkCodec::Result* result,
                                       SkCodecs::DecodeContext ctx) {
    return Decode(sk_sp<const SkData>(static_cast<const SkData*>(data.release())), result, ctx);
}
```

Good:

```cpp
// Canonical function in header has default argument '= nullptr':
SK_API std::unique_ptr<SkCodec> Decode(sk_sp<const SkData> data,
                                       SkCodec::Result* result,
                                       SkCodecs::DecodeContext ctx = nullptr);

// Compatibility overload replicates '= nullptr' so 2-argument callers continue resolving:
inline std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                       SkCodec::Result* result,
                                       SkCodecs::DecodeContext ctx = nullptr) {
    return Decode(sk_sp<const SkData>(static_cast<const SkData*>(data.release())), result, ctx);
}
```
