---
name: skia/general/cast-signed-integers-before-shifting-or-packing
rubric: correctness
severity: blocking
---

**Rule.** Do not left-shift or bitwise-pack signed integer values without first
converting them to an unsigned type of the appropriate width (such as uint16_t
or uint32_t). When packing multiple fields into an integer, explicitly cast
signed values before shifting or bitwise ORing to prevent undefined behavior
from negative shifts and sign-extension corruption.

**Why.** In C++, left-shifting negative signed integers is undefined behavior or
overflows into the sign bit. Furthermore, bitwise ORing a negative signed
integer causes sign extension across the entire width of the promoted integer
type, overwriting and corrupting higher-order bits packed by preceding shifts.

## Examples

Bad:

```cpp
// fLeft and fTop are int16_t and can be negative
buffer.writeUInt(fLeft << 16 | fTop);
```

Good:

```cpp
// Cast to uint16_t to avoid undefined behavior on negative shifts and prevent sign extension
const uint32_t left = static_cast<uint16_t>(fLeft);
const uint32_t top = static_cast<uint16_t>(fTop);
buffer.writeUInt(left << 16 | top);
```
