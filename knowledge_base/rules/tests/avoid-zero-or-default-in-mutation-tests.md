---
name: skia/tests/avoid-zero-or-default-in-mutation-tests
description: When testing getters, accessors, or state mutations in unit tests, assert exact values over truthiness or omitted checks, verify setters via readback, and avoid default sentinels
rubric: testing
severity: should_fix
---

**Rule.** When testing getters, accessors, setters, or state mutations in unit
tests, assert exact expected values rather than loose truthiness, non-null
checks, or omitting assertions on called accessors that lack dedicated test
coverage. Verify mutator or setter calls by reading back updated values with
corresponding getters. Additionally, use distinct, non-zero, and non-default
values for setter inputs rather than 0, false, or default sentinels so
assertions verify that the operation actually mutated target state rather than
matching pre-existing or zero-initialized memory.

**Why.** Asserting mere truthiness or existence on getters allows wrong default
values, unit mismatches, or unexpected scales to pass silently, while omitting
assertions on accessors leaves their behavior unverified. Failing to read back
state leaves setter updates unverified, and using zero or default sentinels as
inputs can cause assertions to pass accidentally against uninitialized,
zero-cleared, or pre-existing state without verifying that the setter executed
properly.

## Examples

Bad:

```cpp
// Bad: Omitting assertions on accessors used only for intermediate test setup
const text = builder.getText();
const bidis = CanvasKit.Malloc(Uint32Array, 3);
bidis.toTypedArray().set([0, text.length, 0]);

// Bad: Checking only truthiness or omitting getter readback after mutating state
expect(animation.getScalarSlot('Opacity')).toBeTruthy();
animation.setColorSlot('FillsGroup', CanvasKit.RED);
animation.setScalarSlot('Opacity', 0);

// Bad: Mutating with 0 or default sentinels can match pre-existing zero-initialized state
expect(animation.setScalarSlot('Opacity', 0)).toBeTruthy();
expect(animation.getScalarSlot('Opacity')).toBe(0);

// Bad (C++): Setting default 0 or false fails to prove the setter mutated state
paint.setAlpha(0);
REPORTER_ASSERT(reporter, paint.getAlpha() == 0);
```

Good:

```cpp
// Good: Assert exact return values on called accessors
const text = builder.getText();
expect(text).toEqual('The quick brown fox\nate a hamburgerfons and got sick.');
const bidis = CanvasKit.Malloc(Uint32Array, 3);
bidis.toTypedArray().set([0, text.length, 0]);

// Good: Assert exact initial value and read back mutated values with getters
expect(animation.getScalarSlot('Opacity')).toBe(100);
animation.setColorSlot('FillsGroup', CanvasKit.RED);
expect(animation.getColorSlot('FillsGroup')).toBe(CanvasKit.RED);

// Good: Use distinct, non-zero, non-default values to prove active state mutation
expect(animation.setScalarSlot('Opacity', 25)).toBeTruthy();
expect(animation.getScalarSlot('Opacity')).toBe(25);

// Good (C++): Use distinct, non-default value to verify the update succeeded
paint.setAlpha(128);
REPORTER_ASSERT(reporter, paint.getAlpha() == 128);
```
