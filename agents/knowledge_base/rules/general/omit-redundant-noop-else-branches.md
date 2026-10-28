---
name: skia/omit-redundant-noop-else-branches
description: When writing conditional statements, omit redundant no-op else branches or empty blocks that perform no useful work or merely append empty strings
rubric: readability
severity: nit
---

**Rule.** Do not retain redundant or no-op else branches that perform no useful
work, such as appending empty strings, executing empty statements, or leaving
empty {} blocks. Omit the else branch entirely when the alternative case
requires no state mutations or side effects.

**Why.** Empty or no-op branches add unnecessary visual clutter and cognitive
overhead without altering program state. Omitting them keeps control flow clean
and prevents readers from wondering whether a fallback operation was
accidentally omitted or intended to execute.

## Examples

Bad:

```cpp
if (instruction.duplicates > 0) {
    SkString dups;
    dups.printf("*%d", instruction.duplicates + 1);
    label.append(dups);
} else {
    label.append("");
}
```

Good:

```cpp
if (instruction.duplicates > 0) {
    label.appendf("*%d", instruction.duplicates + 1);
}
```
