---
name: skia/indent-chopped-function-parameters-eight-spaces
description: When breaking function arguments or parameters across lines, indent continuation lines eight spaces and pack arguments within 100 columns rather than listing one per line
rubric: readability
severity: should_fix
---

**Rule.** When breaking function call arguments or parameter declarations across
lines by placing them on the next line (chopping after the opening parenthesis),
indent continuation lines by eight spaces (two tabs) relative to the function
statement rather than aligning them under the opening parenthesis. Furthermore,
pack multiple arguments per continuation line or group related parameters into
logical subsets that fit within the 100-column limit, rather than placing each
individual argument on its own separate line.

**Why.** Aligning continuation lines under an opening parenthesis consumes
excessive horizontal space when the call or declaration has a long prefix, while
placing each argument on a separate line causes unnecessary vertical sprawl.
Chopping arguments onto eight-space indented continuation lines and packing them
within 100 columns keeps call sites concise and conforms to Skia's style guide.

## Examples

Bad:

```cpp
// Bad: Aligning under opening parenthesis consumes excessive horizontal space.
this->topDevice()->setLocalToDevice(
                      this->topDevice()->localToDevice44() *
                      SkM44(preViewMatrices[imageSet[i].fMatrixIndex]));

// Bad: Mechanically placing each argument on its own line causes vertical sprawl.
return SkTypeface_AndroidNDK::Make(
        proxy,
        SkString(filePath),
        cacheFontFiles,
        SkTo<int>(collectionIndex),
        style,
        proxy->isFixedPitch(),
        familyName,
        std::move(skLangs));
```

Good:

```cpp
// Good: Indent chopped arguments eight spaces (two tabs) from the statement.
this->topDevice()->setLocalToDevice(
        this->topDevice()->localToDevice44() *
        SkM44(preViewMatrices[imageSet[i].fMatrixIndex]));

// Good: Pack multiple arguments per line or group related parameters within 100 columns.
return SkTypeface_AndroidNDK::Make(
        proxy, SkString(filePath), cacheFontFiles, SkTo<int>(collectionIndex),
        style, proxy->isFixedPitch(), familyName, std::move(skLangs));
```
