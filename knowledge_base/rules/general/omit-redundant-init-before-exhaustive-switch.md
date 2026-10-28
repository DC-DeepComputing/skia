---
name: skia/omit-redundant-init-before-exhaustive-switch
description: When declaring local variables populated by an exhaustive switch statement or one with a default case, omit redundant variable initializers at declaration
rubric: readability
severity: nit
---

**Rule.** Do not redundantly initialize a local variable at declaration when it
is unconditionally assigned in every case or default branch of a subsequent
switch statement. Leave the local variable uninitialized at declaration so each
branch explicitly defines its value and dead stores are avoided.

**Why.** Initializing a local variable to a dummy or default value that is
immediately overwritten across all branches of a switch statement creates dead
stores and visual clutter. Redundant pre-initialization can also mislead readers
into believing the initial value might survive the switch or mask unhandled
cases.

## Examples

Bad:

```cpp
// Bad: Redundant initial value is immediately overwritten in every switch branch
int android_priority = ANDROID_LOG_DEBUG;
switch (priority) {
    case SkLogPriority::kFatal:   android_priority = ANDROID_LOG_FATAL;   break;
    case SkLogPriority::kError:   android_priority = ANDROID_LOG_ERROR;   break;
    default:                      android_priority = ANDROID_LOG_DEBUG;   break;
}
__android_log_vprint(android_priority, LOG_TAG, format, args);
```

Good:

```cpp
// Good: Omit dead store initialization; all switch cases explicitly assign the variable
int android_priority;
switch (priority) {
    case SkLogPriority::kFatal:   android_priority = ANDROID_LOG_FATAL;   break;
    case SkLogPriority::kError:   android_priority = ANDROID_LOG_ERROR;   break;
    default:                      android_priority = ANDROID_LOG_DEBUG;   break;
}
__android_log_vprint(android_priority, LOG_TAG, format, args);
```
