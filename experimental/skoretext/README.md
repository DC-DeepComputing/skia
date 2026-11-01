# SkoreText measurement programs (DO NOT SUBMIT)

These programs back the numbers in the SkoreText design doc. They are not part of any build
target. The standalone CoreText programs need only macOS (the native probes also build for the
iOS simulator); the SkParagraph programs link against a Skia build of the SkoreText CL stack.
Section numbers refer to design doc v13 and are used only for parts that do not discuss
`letterSpacing`; the `letterSpacing` and `trak` parts are named by topic because the doc is being
renumbered there. All commands run from the Skia checkout root, in bash or zsh. The `.patch` file
applies to stack top 0000ea03ab (`[SkoreText] [viewer] Add ParagraphBugsSlide comparison viewer`).

## Programs

- `ct_relayout_cost.c` (section 2, table "cost of one new width"): CoreText cost of a new width
  with a new `CTTypesetter`, with a kept `CTTypesetter` (`CTTypesetterSuggestLineBreak` +
  `CTTypesetterCreateLine`), and one `CTLine` for the whole text. Pass `Helvetica` as the doc
  does: without an argument it uses the system UI font (`.SFNS`), and a new `CTTypesetter` per
  width costs 0.66 ms instead of 0.28 ms on this text.
- `skpar_relayout_bench.cpp` (section 2, same table; the cost of `letterSpacing`; section 6,
  item 6, first `layout` numbers): SkParagraph first layout and re-layout at a new width,
  `kCoreText` + `SkUnicode_apple` vs `kDefault` + ICU, `ParagraphCache` off. Arguments: font
  family, `letterSpacing`, and optional `loop` (shapes `kCoreText` for 8 s, for a profiler).
- `prof_shares.py` (section 2, shares of the first `kCoreText` layout; section 6, item 6):
  inclusive sample shares per pipeline stage from an `xctrace` Time Profiler export of
  `skpar_relayout_bench ... loop`.
- `skpar_fallback_trak_bench.cpp` (cost of `letterSpacing` with fallback fonts): first
  `kCoreText` layout of one 17 pt `.AppleSystemUIFont` line, Latin and Latin + CJK + emoji +
  Hebrew, `letterSpacing` 0 and 1, `ParagraphCache` off. Every line is typeset once.
- `trak_probe.m` (native `letterSpacing` semantics: kern and tracking stack on `trak`): for
  `systemFontOfSize` at 11, 13, 17, 28 and 48 pt, Latin, CJK, Hebrew and mixed text, and spacing
  0, -0.41, 1 and 5, it prints the `CTLine` width with `NSKernAttributeName` and
  `NSTrackingAttributeName` (and the same through `boundingRect` and `UILabel` / `NSTextField`)
  next to two models: `e_stack` (plain width + spacing per grapheme) and `d_skt` (every run font
  copied with `NSCTFontUnscaledTrackingAttribute=0`, then + spacing per grapheme, the earlier
  SkoreText scheme in which spacing replaced `trak`).
- `swiftui_probe.swift` (same claim, SwiftUI): `Text.kerning` and `Text.tracking` widths through
  `UIHostingController`, for comparison with the `UILabel` column of `trak_probe`.
- `liga_probe.c` (native tracking drops ligatures for any nonzero value): glyph counts of "fi",
  "ffi" and "office fluffy" at 24 pt in 10 fonts under `kCTTrackingAttributeName` and
  `kCTKernAttributeName` from -2 to 1, then glyph positions of combining mark, Hebrew point and
  ZWJ emoji clusters under tracking 1 and kern 1 at 17 pt.
- `ascii_fast_path.patch` (section 6, item 6): prototype printable-ASCII fast path in
  `SkUnicode_apple` (env `R49_UNI_FAST=1`).
- `unicode_flags_dump.cpp` (section 6, item 6): dumps `SkUnicode_apple` code unit flags, so runs
  with and without the prototype diff byte for byte.

## Standalone CoreText programs

```bash
mkdir -p /tmp/r491_bin
for p in ct_relayout_cost liga_probe; do
  clang -O2 -framework CoreText -framework CoreFoundation experimental/skoretext/$p.c \
    -o /tmp/r491_bin/$p
done
/tmp/r491_bin/ct_relayout_cost Helvetica
/tmp/r491_bin/liga_probe
```

## Native tracking probes (macOS and iOS simulator)

```bash
clang -O2 -fobjc-arc -framework CoreText -framework AppKit experimental/skoretext/trak_probe.m \
  -o /tmp/r491_bin/trak_probe_mac
/tmp/r491_bin/trak_probe_mac

SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)
clang -O2 -fobjc-arc -isysroot "$SDK" -target arm64-apple-ios26.0-simulator \
  -framework CoreText -framework UIKit experimental/skoretext/trak_probe.m \
  -o /tmp/r491_bin/trak_probe_ios
xcrun --sdk iphonesimulator swiftc -O -target arm64-apple-ios26.0-simulator \
  experimental/skoretext/swiftui_probe.swift -o /tmp/r491_bin/swiftui_probe
xcrun simctl boot "iPhone 17 Pro"
xcrun simctl spawn booted /tmp/r491_bin/trak_probe_ios
xcrun simctl spawn booted /tmp/r491_bin/swiftui_probe
xcrun simctl shutdown "iPhone 17 Pro"
```

## SkParagraph programs

Build the stack with CoreText shaping and both `SkUnicode` implementations:

```bash
mkdir -p out/AppleUnicode
printf 'is_debug = false\nskia_use_coretext_paragraph = true\nskia_use_apple_unicode = true\n' \
  > out/AppleUnicode/args.gn
bin/gn gen out/AppleUnicode
third_party/ninja/ninja -C out/AppleUnicode skparagraph skshaper skunicode_apple skunicode_icu skia
```

Link the benches and the flags dump:

```bash
O=out/AppleUnicode
for p in skpar_relayout_bench unicode_flags_dump skpar_fallback_trak_bench; do
  clang++ -std=c++20 -O2 -DNDEBUG -DSK_ENABLE_PARAGRAPH -DSK_PARAGRAPH_NATIVE_CORETEXT \
    -DSK_UNICODE_AVAILABLE -DSK_UNICODE_ICU_IMPLEMENTATION -DSK_UNICODE_APPLE_IMPLEMENTATION -I. \
    experimental/skoretext/$p.cpp -o /tmp/r491_bin/$p \
    $O/lib{skparagraph,skshaper,skunicode_apple,skunicode_icu,skunicode_core,skia}.a \
    $O/lib{harfbuzz,icu,icu_bidi,skcms,png,jpeg,webp,webp_sse41,expat}.a \
    -framework CoreText -framework CoreGraphics -framework CoreFoundation -framework ImageIO \
    -framework Foundation -framework OpenGL -framework AppKit -framework Metal
done
/tmp/r491_bin/skpar_relayout_bench Helvetica 0   # letterSpacing 0
/tmp/r491_bin/skpar_relayout_bench Helvetica 1   # letterSpacing 1
/tmp/r491_bin/skpar_fallback_trak_bench
```

Profile the first layout of `kCoreText`:

```bash
xcrun xctrace record --template 'Time Profiler' --output /tmp/prof.trace \
  --launch -- /tmp/r491_bin/skpar_relayout_bench Helvetica 0 loop
xcrun xctrace export --input /tmp/prof.trace \
  --xpath '/trace-toc/run[@number="1"]/data/table[@schema="time-profile"]' --output /tmp/prof.xml
python3 experimental/skoretext/prof_shares.py /tmp/prof.xml
```

ASCII fast path prototype: `git apply experimental/skoretext/ascii_fast_path.patch`, rebuild,
relink, then run the bench or `unicode_flags_dump` with and without `R49_UNI_FAST=1`.

## Reference results

Apple M5 Max, macOS 26.7.1, stack top 0000ea03ab, Nov 1 2026, load average 3.7 to 4.1 (1 min).

- `skpar_relayout_bench Helvetica 0`, median of 5 runs, each a mean over 200 iterations:
  `kCoreText` first layout 0.753 ms, re-layout 0.038 ms; `kDefault` 0.525 / 0.038 ms.
- `skpar_relayout_bench Helvetica 1`, same: `kCoreText` first layout 0.778 ms, re-layout
  0.040 ms; `kDefault` 0.565 / 0.040 ms. The text has 40 CJK runs that fall back to PingFang.
  When spacing replaced `trak` (stack top d277c27e9f, Oct 31), fallback runs were typeset a
  second time and `kCoreText` first layout took 1.238 ms.
- `skpar_fallback_trak_bench`, median of 3 runs, each the best of 5 x 2000 layouts:

  | Text                         | First `kCoreText` layout | Oct 31, second pass on |
  |------------------------------|--------------------------|------------------------|
  | Latin, letterSpacing 0       | 49.4 us                  | 51.1 us                |
  | Latin, letterSpacing 1       | 54.7 us                  | 62.2 us                |
  | Latin+CJK+emoji+Hebrew, ls 0 | 119.4 us                 | 124.6 us               |
  | Latin+CJK+emoji+Hebrew, ls 1 | 131.0 us                 | 249.2 us               |

- `ct_relayout_cost Helvetica` (CoreText only, so the stack does not change it; Oct 31 numbers
  confirmed on Nov 1 within 0.01 ms): typesetter create 0.178 ms, typesetter + endless `CTLine`
  0.189, kept typesetter Suggest 0.071 and Suggest + `CTLine`s 0.108, new typesetter 0.278 ms per
  width.
- `ct_relayout_cost` (`.SFNS`, Oct 31): create 0.553 ms, endless 0.564, kept 0.071 / 0.107, new
  typesetter 0.660 ms per width.
- `trak_probe`: in every row with nonzero spacing, `kern - e_stack` and `track - e_stack` are
  0.000, on macOS and in the iOS 26.4 simulator (the two outputs match digit for digit, Oct 31);
  `kern - d_skt` reaches 4.748. `boundingRect` gives the same widths; `UILabel`, `NSTextField`
  and `swiftui_probe` give them rounded up to 1/3 pt. So native kern and tracking add to `trak`,
  for `.SFNS` and for the fallback fonts (PingFang, SF Hebrew).
- `liga_probe`: `kCTTrackingAttributeName` drops ligatures for every nonzero value, -0.01 and
  0.01 included (Helvetica "office fluffy": 11 glyphs without tracking, 13 with it; same in Times,
  Hoefler Text, Baskerville, Didot, Helvetica Neue, Palatino). `kCTKernAttributeName` keeps them,
  except Hoefler Text at -2 (12 glyphs instead of 9). Tracking 1 adds 1 pt once per grapheme
  cluster: "a e U+0301 x" +3, Hebrew with 2 points +4, man ZWJ woman + "a" +2.
