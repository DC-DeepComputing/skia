# SkoreText measurement programs (DO NOT SUBMIT)

These programs back the numbers in the SkoreText design doc. They are not part of any build
target. The standalone CoreText programs need only macOS; the SkParagraph programs link against
a Skia build of the SkoreText CL stack. Section numbers refer to design doc v12. All commands
run from the Skia checkout root, in bash or zsh.

## Programs

- `ct_relayout_cost.c` (section 2, table "cost of one new width"): CoreText cost of a new width
  with a new `CTTypesetter`, with a kept `CTTypesetter` (`CTTypesetterSuggestLineBreak` +
  `CTTypesetterCreateLine`), and one `CTLine` for the whole text. Pass `Helvetica` as the doc
  does: without an argument it uses the system UI font (`.SFNS`), and a new `CTTypesetter` per
  width costs 0.66 ms instead of 0.28 ms on this text.
- `skpar_relayout_bench.cpp` (section 2, same table, and the first `layout` numbers in section
  6, item 4): SkParagraph first layout and re-layout at a new width, `kCoreText` +
  `SkUnicode_apple` vs `kDefault` + ICU, `ParagraphCache` off. Arguments: font family,
  `letterSpacing`, and optional `loop` (shapes `kCoreText` for 8 s, for a profiler).
- `prof_shares.py` (section 2, shares of the first `kCoreText` layout; section 6, item 4):
  inclusive sample shares per pipeline stage from an `xctrace` Time Profiler export of
  `skpar_relayout_bench ... loop`.
- `ct_fallback_trak_v2.c` (section 6, item 1): the current `letterSpacing` scheme (no
  `kCTTrackingAttributeName`, primary font copied with `NSCTFontUnscaledTrackingAttribute=0`).
  For each script, size and primary font (`.SFNS`, Helvetica) it prints the fallback run width
  with the plain primary (`plain`), with the attributed primary (`current`), the fallback font
  copied with the attribute and typeset alone (`target`), and after the second pass (`fixB`).
  `plain == current` shows the attribute does not reach fallback fonts; `fixB == target` shows
  that a second pass, as in `CoreTextLineShaper.cpp`, fixes it.
- `ct_fallback_trak_control.c` (section 6, item 1, which fallback fonts have a `trak` table):
  each fallback font used directly, with its `trak` table presence and the width change from the
  attribute. Uses no tracking, so the result does not depend on the `letterSpacing` scheme.
- `ct_fallback_trak_fix.c` (section 6, item 1, rejected fix): fix A (attribute on every
  `kCTFontCascadeListAttribute` descriptor) vs fix B (second pass). The argument sets
  `kCTTrackingAttributeName` (default 2, the old scheme); `0` is the current scheme. Fix A changes
  no width under either scheme.
- `ct_second_pass_cost.c` (section 6, item 1, cost of the fix): splits the second pass into
  copying the fallback fonts with the attribute, a second `CTTypesetter` + `CTLine`, and the
  mutable copy of the attributed string, for one 17 pt line with CJK, emoji and Hebrew.
- `ascii_fast_path.patch` (section 6, item 4): prototype printable-ASCII fast path in
  `SkUnicode_apple` (env `R49_UNI_FAST=1`).
- `unicode_flags_dump.cpp` (section 6, item 4): dumps `SkUnicode_apple` code unit flags, so runs
  with and without the prototype diff byte for byte.

## Standalone CoreText programs

```bash
mkdir -p /tmp/r491_bin
for p in ct_relayout_cost ct_fallback_trak_v2 ct_fallback_trak_control ct_fallback_trak_fix \
         ct_second_pass_cost; do
  clang -O2 -framework CoreText -framework CoreFoundation experimental/skoretext/$p.c \
    -o /tmp/r491_bin/$p
done
/tmp/r491_bin/ct_relayout_cost Helvetica
/tmp/r491_bin/ct_fallback_trak_v2
/tmp/r491_bin/ct_fallback_trak_control
/tmp/r491_bin/ct_fallback_trak_fix 0   # current scheme; no argument = old scheme, tracking 2
/tmp/r491_bin/ct_second_pass_cost
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

Link the bench and the flags dump:

```bash
O=out/AppleUnicode
for p in skpar_relayout_bench unicode_flags_dump; do
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

Apple M5 Max, macOS 26.7.1, stack top d277c27e9f (`[SkoreText] [viewer] Add
ParagraphBugsSlide comparison viewer`), Oct 31 2026, load average 3.1 (1 min). Median of 3 runs;
each run is a mean over 200 iterations (2000 for `ct_second_pass_cost`).

- `skpar_relayout_bench Helvetica 0`: `kCoreText` first layout 0.773 ms, re-layout 0.038 ms;
  `kDefault` 0.520 / 0.038 ms.
- `skpar_relayout_bench Helvetica 1`: `kCoreText` first layout 1.238 ms, re-layout 0.040 ms;
  `kDefault` 0.563 / 0.040 ms.
- `ct_relayout_cost Helvetica`: typesetter create 0.178 ms, typesetter + endless `CTLine` 0.189,
  kept typesetter Suggest 0.071 and Suggest + `CTLine`s 0.108, new typesetter 0.278 ms per width.
- `ct_relayout_cost` (`.SFNS`): create 0.553 ms, endless 0.564, kept 0.071 / 0.107, new
  typesetter 0.660 ms per width.
- `ct_second_pass_cost`: 3 fallback fonts, font copies 9.4 us, second typeset 74.2 us, mutable
  copy of the string 0.34 us.

On Oct 30, on the stack without the fallback trak second pass, `kCoreText` with
`letterSpacing 1` took 0.847 ms. The text has 40 CJK runs that fall back to PingFang, so with
`letterSpacing != 0` it is now typeset twice. The `letterSpacing 0` numbers did not change beyond
noise (0.738 ms then, 0.737 to 0.793 ms now).

`ct_fallback_trak_v2`: `plain == current` and `fixB == target` in all 90 rows. With `.SFNS` at
17 pt, `current - target` is +0.943 (CJK), -1.240 (Kana), -1.295 (Hebrew, Armenian), -0.323
(Devanagari) and +4.000 (emoji); Hangul, Arabic and Thai are 0. With Helvetica only emoji
differs, +2 pt at 9 to 24 pt.

`ct_fallback_trak_fix 0` and `ct_fallback_trak_fix`: fix A equals `current` and fix B equals
`target` in all 8 cases under both schemes.
