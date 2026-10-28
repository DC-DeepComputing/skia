---
name: skia/ports/pass-variation-position-to-font-arguments
description: When instantiating typefaces or proxies using SkFontArguments in font managers, pass matched collection indices via setCollectionIndex and variation coordinates via setVariationDesignPosition
rubric: correctness
severity: should_fix
---

**Rule.** When constructing a typeface or typeface proxy from a stream, font
scanner, or descriptor pattern using SkFontArguments (such as in FontConfig or
Android font managers), extract and pass the font collection index (such as
FontConfig's FC_INDEX) via setCollectionIndex() and convert requested variation
coordinates (such as FontConfig's FC_VARIABLE and FC_FONT_VARIATIONS or parsed
scanner axes) via setVariationDesignPosition(). Do not default-construct
SkFontArguments with index 0 when querying multi-face font collections (.ttc,
.otc), nor omit parsed variation coordinates; if temporarily deferring variation
support during incremental refactoring, add an explicit TODO comment tracking
omitted arguments.

**Why.** Default-constructing SkFontArguments selects collection index 0, which
incorrectly chooses the first face rather than the specific font face identified
by the FontConfig pattern or font collection index in multi-face font files
(.ttc, .otc). Furthermore, omitting setVariationDesignPosition() causes variable
fonts to instantiate with default axis coordinates, discarding custom weight,
width, slant, or FontConfig variation settings.

## Examples

Bad:

```cpp
// 1. FontConfig: Default SkFontArguments ignores FC_INDEX and variation properties
int ttcIndex = get_int(pattern, FC_INDEX, 0);
fProxy = SkTypeface_FreeType::MakeFromStream(
        SkStream::MakeFromFile(filename), SkFontArguments());

// 2. Android / Scanner: Omitting parsed axis coordinates causes default axes
auto proxy = scanner->MakeFromStream(
        std::move(stream),
        SkFontArguments().setCollectionIndex(collectionIndex));
```

Good:

```cpp
// 1. FontConfig: Pass collection index and document deferred variation properties
int ttcIndex = get_int(pattern, FC_INDEX, 0);
// TODO: FC_VARIABLE and FC_FONT_VARIATIONS in arguments
fProxy = SkTypeface_FreeType::MakeFromStream(
        SkStream::MakeFromFile(filename),
        SkFontArguments().setCollectionIndex(ttcIndex));

// 2. Android / Scanner: Pass both collection index and variation coordinates
SkFontArguments::VariationPosition position = {
        requestAxisValues.get(), SkTo<int>(requestAxisCount)};
auto proxy = scanner->MakeFromStream(
        std::move(stream),
        SkFontArguments()
                .setCollectionIndex(collectionIndex)
                .setVariationDesignPosition(position));
```
