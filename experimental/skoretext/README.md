# SkoreText measurement programs (DO NOT SUBMIT)

These programs back the numbers in the SkoreText design doc. They are not part of any build
target. The standalone CoreText programs need only macOS; the SkParagraph programs link against
an existing Skia build of the SkoreText CL stack.

- `ct_relayout_cost.c`: cost of one new width in CoreText (new `CTTypesetter`; kept
  `CTTypesetter` with `CTTypesetterSuggestLineBreak` + `CTTypesetterCreateLine`; one `CTLine`
  for the whole text).
- `skpar_relayout_bench.cpp`: SkParagraph first layout and re-layout at a new width,
  `kCoreText` + `SkUnicode_apple` vs `kDefault` + ICU, optional `letterSpacing`.
- `prof_shares.py`: inclusive sample shares per pipeline stage from an `xctrace` Time Profiler
  export of the bench.
- `ct_fallback_trak.c`: whether `NSCTFontUnscaledTrackingAttribute=0` on the primary font
  reaches cascade fallback fonts.
- `ct_fallback_trak_control.c`: control, `trak` effect of each fallback font used directly.
- `ct_fallback_trak_fix.c`: two candidate fixes, a no-`trak` cascade list vs a second pass that
  pins fallback runs.
- `ascii_fast_path.patch`: prototype printable-ASCII fast path in `SkUnicode_apple`
  (env `R49_UNI_FAST=1`).
- `unicode_flags_dump.cpp`: dumps `SkUnicode_apple` code unit flags, so runs with and without
  the prototype diff byte for byte.

## Standalone CoreText programs

```bash
cd experimental/skoretext
for p in ct_relayout_cost ct_fallback_trak ct_fallback_trak_control ct_fallback_trak_fix; do
  clang -O2 -framework CoreText -framework CoreFoundation -framework CoreGraphics $p.c -o /tmp/$p
done
/tmp/ct_relayout_cost
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

Link the bench (same for `unicode_flags_dump.cpp`):

```bash
O=out/AppleUnicode
L="skparagraph skshaper skunicode_apple skunicode_icu skunicode_core skia harfbuzz icu icu_bidi
   skcms png jpeg webp webp_sse41 expat perfetto"
clang++ -std=c++20 -O2 -DNDEBUG -DSK_ENABLE_PARAGRAPH -DSK_PARAGRAPH_NATIVE_CORETEXT \
  -DSK_UNICODE_AVAILABLE -DSK_UNICODE_ICU_IMPLEMENTATION -DSK_UNICODE_APPLE_IMPLEMENTATION -I. \
  experimental/skoretext/skpar_relayout_bench.cpp -o /tmp/skpar_relayout_bench \
  $(for l in $L; do echo $O/lib$l.a; done) \
  -framework CoreText -framework CoreGraphics -framework CoreFoundation -framework ImageIO \
  -framework Foundation -framework OpenGL -framework AppKit -framework Metal
/tmp/skpar_relayout_bench Helvetica 0   # letterSpacing 0
/tmp/skpar_relayout_bench Helvetica 1   # letterSpacing 1
```

Profile the first layout of `kCoreText` (the `loop` mode shapes for 8 s):

```bash
xcrun xctrace record --template 'Time Profiler' --output /tmp/prof.trace \
  --launch -- /tmp/skpar_relayout_bench Helvetica 0 loop
xcrun xctrace export --input /tmp/prof.trace \
  --xpath '/trace-toc/run[@number="1"]/data/table[@schema="time-profile"]' --output /tmp/prof.xml
python3 experimental/skoretext/prof_shares.py /tmp/prof.xml
```

ASCII fast path prototype: `git apply experimental/skoretext/ascii_fast_path.patch`, rebuild,
then run the bench or `unicode_flags_dump` with and without `R49_UNI_FAST=1`.

Measure on an idle machine: the numbers in the doc are means over 200 iterations on Apple
Silicon, macOS 26.7.1.
