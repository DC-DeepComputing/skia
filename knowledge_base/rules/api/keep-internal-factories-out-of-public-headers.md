---
name: skia/api/keep-internal-factories-out-of-public-headers
description: When declaring factory functions in public headers under include/, keep internal helpers in private headers and define concrete factory classes in .cpp files
rubric: design
severity: should_fix
---

**Rule.** Do not expose internal, bindings-specific, or consumer-specific
factory methods in public module or library headers under include/; place
internal helpers, test hooks, and bindings utilities in private headers under
src/ or in dedicated component headers. Additionally, do not define concrete
implementation classes or inline non-trivial factory functions in public
headers; declare the factory function out-of-line (annotated with SK_API or
module export macros) and define the concrete implementation class and factory
body in the corresponding .cpp source file.

**Why.** Public headers in include/ and modules/**/include/ define Skia's
client-facing API surface. Exposing internal or bindings-only factories pollutes
public interfaces, while inlining factory methods or defining concrete
implementation classes in headers exposes internal details, forces unnecessary
header dependencies onto clients, and increases compilation overhead.

## Examples

Bad:

```cpp
// modules/skunicode/include/SkUnicode_client.h (internal/bindings factory in general header)
namespace SkUnicodes::Client {
SKUNICODE_API sk_sp<SkUnicode> Make(SkSpan<char> text, ...);
SKUNICODE_API sk_sp<SkUnicode> Make(); // Internal bidi-only factory
}

// modules/skshaper/include/SkShaper_factory.h (concrete class & inline factory in public header)
namespace priv {
class PrimitiveFactory : public Factory {
    std::unique_ptr<SkShaper> makeShaper(sk_sp<SkFontMgr>) const override { ... }
};
}
namespace Primitive {
inline sk_sp<Factory> Factory() { return sk_make_sp<SkShapers::priv::PrimitiveFactory>(); }
}
```

Good:

```cpp
// modules/skunicode/include/SkUnicode_client.h (dedicated component header or src/ for bidi)
namespace SkUnicodes::Client {
SKUNICODE_API sk_sp<SkUnicode> Make(SkSpan<char> text, ...);
}
// Dedicated: modules/skunicode/include/SkUnicode_bidi.h (or private header under src/)
namespace SkUnicodes::Bidi { SKUNICODE_API sk_sp<SkUnicode> Make(); }

// modules/skshaper/include/SkShaper_factory.h (out-of-line factory declaration)
namespace Primitive {
SKSHAPER_API sk_sp<Factory> Factory();
}
// modules/skshaper/src/SkShaper_factory.cpp (concrete class & definition in .cpp)
namespace {
class PrimitiveFactory final : public SkShapers::Factory { ... };
}
namespace SkShapers::Primitive {
sk_sp<SkShapers::Factory> Factory() { return sk_make_sp<PrimitiveFactory>(); }
}
```
