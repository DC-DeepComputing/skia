---
name: skia/site/distinguish-blink-reference-tests-in-roll-docs
description: When documenting Chromium roll procedures and test triage, distinguish Blink reference tests from image tests and identify non-layout image tests as pixel tests
rubric: readability
severity: should_fix
---

**Rule.** When documenting Chromium roll procedures or Blink layout test
rebaselining, distinguish reference tests (comparing .html to -expected.html)
from image-based layout tests, noting that automated tooling (such as
rebaseline-cl) only updates reference images while reference test failures must
be resolved manually by updating HTML or configuring <meta name="fuzzy"> tags.
Additionally, when documenting non-layout tests that compare against checked-in
expected images (such as browser or views tests), refer to them as 'pixel tests'
(Chromium's standard terminology).

**Why.** Blink layout tests and non-layout pixel tests follow distinct triage
and rebaselining workflows in Chromium. Automated tools like rebaseline-cl only
update layout PNG baselines and cannot resolve HTML reference tests, while
non-layout tests verifying checked-in images are standardly called pixel tests.
Using standard terminology prevents confusion when developers search for failure
logs or triage roll breaks.

## Examples

Bad:

```cpp
* Create Chromium CL to update expectations. Follow rebaselining steps to rebaseline all failing layout tests.
* Some non-layout tests will fail as a result of rendering changes because they have their own checked-in images.
```

Good:

```cpp
* Create Chromium CL to update expectations. Follow rebaselining steps for layout tests that use reference images; for reference tests (.html vs. -expected.html), manually update the HTML or adjust <meta name="fuzzy"> tags.
* Some non-layout tests (also called pixel tests) will fail as a result of rendering changes because they have their own checked-in images.
```
