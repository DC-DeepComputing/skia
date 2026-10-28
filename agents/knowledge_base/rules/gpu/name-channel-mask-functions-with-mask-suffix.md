---
name: skia/gpu/name-channel-mask-functions-with-mask-suffix
description: When declaring functions or accessors that return color channel bitmasks, name them with a Mask suffix rather than plural nouns like Channels
rubric: readability
severity: should_fix
---

**Rule.** Name functions and accessors that return a bitmask or bitfield of
color channels with a '*Mask' or '*ChannelMask' suffix (e.g.,
'TextureFormatChannelMask', 'channelMask()') rather than plural nouns like
'*Channels'. Document that the return value is a bitfield of
'SkColorChannelFlag' values.

**Why.** Function names ending in plural nouns like '*Channels' that return an
integer are easily misread as returning a channel count (such as 3 for RGB or 4
for RGBA) or a collection of channels. Using a '*Mask' suffix clarifies that the
integer represents a bitfield of channel flags.

## Examples

Bad:

```cpp
// Ambiguous whether it returns a channel count (e.g., 4) or a bitmask
uint32_t TextureFormatChannels(TextureFormat);
```

Good:

```cpp
// Returns a bitmask of SkColorChannelFlag values.
uint32_t TextureFormatChannelMask(TextureFormat);
```
