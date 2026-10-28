---
name: skia/resources/match-compiler-diagnostics-in-sksl-error-tests
description: When writing SkSL error test expectations in resources/sksl/errors/, match the compiler's actual diagnostic text between /*%%* delimiters instead of paraphrased descriptions
rubric: correctness
severity: blocking
---

**Rule.** In SkSL error and runtime error test shaders (under
resources/sksl/errors/ and resources/sksl/runtime_errors/), write the compiler's
exact diagnostic messages between the /*%%* and *%%*/ expectation delimiters. Do
not write informal, paraphrased, or high-level descriptions of the error
condition, as SkSLErrorTest verifies expected errors via strict substring
matching against actual compiler output.

**Why.** SkSLErrorTest parses lines within the /*%%* ... *%%*/ block in error
test shaders and scans the compiler's emitted diagnostics in order using
substring matching. Providing informal summaries or paraphrased descriptions
rather than the compiler's actual error text causes the test runner's substring
check to fail.

## Examples

Bad:

```cpp
// resources/sksl/errors/InterfaceBlockMixedCommasAndSemicolons.sksl
uniform foo { int x;, y;, z; };

/*%%*
Mixed ';,' is not permitted on an interface block field
*%%*/
```

Good:

```cpp
// resources/sksl/errors/InterfaceBlockMixedCommasAndSemicolons.sksl
uniform foo { int x;, y;, z; };

/*%%*
expected a type, but found ','
*%%*/
```
