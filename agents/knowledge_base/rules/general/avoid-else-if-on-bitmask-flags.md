---
name: skia/avoid-else-if-on-bitmask-flags
description: When inspecting or formatting bitmask flags, test bits with independent if statements, order checks by bit layout, and accumulate labels into a single string
rubric: readability
severity: should_fix
---

**Rule.** Do not inspect, format, or dispatch on bitmask flags (such as Vulkan
`Vk*Flags` or bitfield enums) using mutually exclusive `if ... else if` chains
when multiple bits can be set simultaneously; test each flag bit independently
in its own `if` statement so that all active flags are handled. Order formatting
checks in a consistent sequence matching the bitfield layout (e.g., MSB to LSB).
Furthermore, when formatting optional flag labels for logging, accumulate active
tags into a single string rather than declaring separate temporary variables per
flag and chaining multiple format specifiers.

**Why.** Bitmask types and bitfield enums represent composable sets where
multiple bits may be set simultaneously; mutually exclusive `else if` branches
silently drop subsequent flags. Checking flags out of order makes formatted
output confusing, while declaring separate temporary variables for every flag
clutters local scope with boilerplate and creates unwieldy format strings with
chained `%s` placeholders.

## Examples

Bad:

```cpp
// Mutually exclusive else-if drops subsequent active flags:
const char* type = "";
if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
    type = "general ";
} else if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    type = "validation ";
}

// Temporary variables per flag and chained format specifiers:
const char* gen = (flags & kGeneral_Flag) ? " <general>" : "";
const char* val = (flags & kValidation_Flag) ? " <validation>" : "";
SkDebugf("Log: %s%s\n", gen, val);

// Mixed flag order (starts with MSB, then ascends from LSB):
// Bitfield: W=bit 4, R=bit 3, L=bit 2, B=bit 1, T=bit 0
if (mask & W) s += "W";
if (mask & T) s += (s.empty() ? "T" : " | T");
```

Good:

```cpp
// Independent if statements handle all active flags:
std::string type;
if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
    type += " <general>";
}
if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
    type += " <validation>";
}
SkDebugf("Log: %s\n", type.c_str());

// Check flags in consistent MSB-to-LSB order matching bit layout:
if (mask & W) s += "W";
if (mask & R) s += (s.empty() ? "R" : " | R");
if (mask & L) s += (s.empty() ? "L" : " | L");
```
