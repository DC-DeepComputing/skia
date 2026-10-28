---
name: skia/prefer-param-comment-empty-braces
description: When declaring default parameter values or passing default-constructed arguments to functions, prefer {} (using /*param=*/{} at call sites) over repeating TypeName()
rubric: readability
severity: nit
---

**Rule.** When declaring default parameter values in function signatures or
passing default-constructed arguments (such as options or info structs) to
functions lacking parameterless overloads, prefer '{}' over repeating the
explicit constructor 'TypeName()'. In parameter declarations, specify '= {}'
directly; at call sites, use '/*paramName=*/{}' to clearly document which
parameter is being value-initialized.

**Why.** Using '{}' eliminates redundant type names and namespace boilerplate
while value-initializing the object. At call sites, combining '{}' with
'/*paramName=*/' keeps the call concise and self-documenting without repeating
verbose constructor expressions.

## Examples

Bad:

```cpp
// Function declaration: redundantly repeating the type in default parameter
void init(DrawBufferManagerOptions dbmOpts = DrawBufferManagerOptions());

// Call site: redundantly constructing the type without inline parameter comment
bool success = fQueueManager->submitToGpu(graphite::SubmitInfo());
```

Good:

```cpp
// Function declaration: clean value-initialization without repeating the type
void init(DrawBufferManagerOptions dbmOpts = {});

// Call site: concise value-initialization with documented parameter name
bool success = fQueueManager->submitToGpu(/*submitInfo=*/{});
```
