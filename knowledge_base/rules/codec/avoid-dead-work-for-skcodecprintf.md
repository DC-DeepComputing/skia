---
name: skia/codec/avoid-dead-work-for-skcodecprintf
description: When preparing or formatting arguments for debug-only SkCodecPrintf logging, evaluate expressions inside the macro call or guard them with SK_PRINT_CODEC_MESSAGES
rubric: safety
severity: should_fix
---

**Rule.** Do not unconditionally prepare arguments, allocate buffers, or invoke
external library functions to format messages for SkCodecPrintf outside the
macro call. Instead, evaluate formatting expressions directly inside the
SkCodecPrintf argument list (using the comma operator if necessary) or enclose
the preparation logic in #if defined(SK_PRINT_CODEC_MESSAGES) so that all
formatting work is elided when codec debug logging is disabled.

**Why.** SkCodecPrintf expands to an empty macro unless SK_PRINT_CODEC_MESSAGES
is explicitly defined during local debugging. Formatting strings or executing
external function pointers outside the macro executes dead work in release
builds, incurs performance overhead during decoding errors, and can trigger
runtime CFI icall violations on uninstrumented shared libraries for output that
is immediately discarded.

## Examples

Bad:

```cpp
static void print_message(const j_common_ptr info, const char caller[]) {
    char buffer[JMSG_LENGTH_MAX];
    info->err->format_message(info, buffer);
    SkCodecPrintf("libjpeg error %d <%s> from %s\n", info->err->msg_code, buffer, caller);
}
```

Good:

```cpp
static void print_message(const j_common_ptr info, const char caller[]) {
    [[maybe_unused]] char buffer[JMSG_LENGTH_MAX];
    SkCodecPrintf("libjpeg error %d <%s> from %s\n",
                  info->err->msg_code,
                  (info->err->format_message(info, buffer), buffer),
                  caller);
}
```
