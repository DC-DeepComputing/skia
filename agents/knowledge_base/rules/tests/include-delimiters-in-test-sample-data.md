---
name: skia/tests/include-delimiters-in-test-sample-data
description: When test fixtures extract tokens, delimiters, or boundary indices from sample data, ensure sample inputs contain target features so extracted collections are not vacuously empty
rubric: testing
severity: should_fix
---

**Rule.** When test fixtures or unit tests extract tokens, delimiters, or
boundary indices from sample data (such as parsing newlines, spaces, or
punctuation from test strings) to pass into an API, ensure the sample data
explicitly contains those target features. Do not omit expected delimiters or
tokens from sample inputs, as this causes the extracted index arrays or
collections to be empty and allows the test to pass vacuously without exercising
the intended feature logic.

**Why.** APIs consuming structured collections (such as line breaks, word
boundaries, or tokens) often handle empty input arrays as valid no-op
configurations. When sample test strings omit the target delimiters, extraction
loops populate zero entries, allowing tests to pass vacuously without exercising
parsing, layout, or feature-handling logic.

## Examples

Bad:

```cpp
// Test text lacks newlines, leaving newLines and hardBreaks empty:
builder.addText('The quick brown fox ate a hamburgerfons and got sick.');
var newLines = [];
for (let i = 0; i < text.length; ++i) {
  i = text.indexOf('\n', i);
  if (i === -1) break;
  newLines.push(i + 1);
}
const hardBreaks = CanvasKit.Malloc(Uint32Array, newLines.length);
hardBreaks.toTypedArray().set(newLines);
builder.buildWithClientInfo(bidis, words, graphemesArr, softBreaks, hardBreaks);
```

Good:

```cpp
// Add newlines to sample text so hardBreaks is populated and tested:
builder.addText('The quick brown fox\nate a hamburgerfons and got sick.');
var newLines = [];
for (let i = 0; i < text.length; ++i) {
  i = text.indexOf('\n', i);
  if (i === -1) break;
  newLines.push(i + 1);
}
const hardBreaks = CanvasKit.Malloc(Uint32Array, newLines.length);
hardBreaks.toTypedArray().set(newLines);
builder.buildWithClientInfo(bidis, words, graphemesArr, softBreaks, hardBreaks);
```
