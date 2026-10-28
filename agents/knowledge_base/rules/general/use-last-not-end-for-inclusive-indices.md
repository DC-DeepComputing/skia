---
name: skia/use-last-not-end-for-inclusive-indices
description: When naming sequence bounds, indices, or iteration state, reserve 'last' for inclusive terminal elements, 'end' for exclusive bounds, and 'prev' for preceding elements
rubric: readability
severity: nit
---

**Rule.** Distinguish inclusive terminal items from exclusive bounds and
preceding elements by reserving 'last...' (such as lastIndex or
lastSwitchCaseIdx) strictly for the final valid element. Use 'end...' (such as
endIndex) strictly for exclusive one-past-the-end boundaries. When iterating
sequences, name state or references to the immediately preceding element
'prev...' or 'previous...' (such as previousCaseFellThrough) rather than
overloading 'last...'.

**Why.** In C++ and standard library conventions, 'end' denotes an exclusive
boundary pointing one past the final element, while 'last' denotes the final
valid item. Overloading 'last' to also represent the immediately preceding item
during iteration creates ambiguity and confusion when tracking both sequence
iteration state and terminal element bounds.

## Examples

Bad:

```cpp
// BAD: 'end...' implies an exclusive upper bound, but points to the final valid item.
int endBufferIdx = bufferOffset + (numStops - 1) * 5;
return $half4_from_array(arr, endBufferIdx + 1);

// BAD: 'lastCaseFellThrough' uses 'last' for the preceding item, conflicting with 'lastSwitchCaseIdx'.
bool lastCaseFellThrough = false;
int lastSwitchCaseIdx = s.cases().size() - 1;
for (int index = 0; index <= lastSwitchCaseIdx; ++index) {
    if (lastCaseFellThrough) { ... }
}
```

Good:

```cpp
// GOOD: 'last...' clearly communicates an inclusive terminal index.
int lastBufferIdx = bufferOffset + (numStops - 1) * 5;
return $half4_from_array(arr, lastBufferIdx + 1);

// GOOD: 'previous...' denotes the preceding element, leaving 'last...' for the terminal element.
bool previousCaseFellThrough = false;
int lastSwitchCaseIdx = s.cases().size() - 1;
for (int index = 0; index <= lastSwitchCaseIdx; ++index) {
    if (previousCaseFellThrough) { ... }
}
```
