# Potential Optimizations for SkScan_Antihair.cpp

This file contains notes on potential performance optimizations for the anti-aliased hairline rasterizer. These are for future consideration and may involve trade-offs with code complexity or require significant changes to the `SkBlitter` API.

### 1. `is_invalid_coord` check

The original `any_bad_ints` function is a branchless sequence of bitwise operations. Replacing it with a more readable series of comparisons (`c == SK_MinS32`) might introduce branches. While modern compilers are excellent at optimizing such patterns, the branchless version could be faster on some architectures. This is a classic readability vs. micro-optimization trade-off that would require profiling to make a definitive choice. For non-critical code, readability is usually preferred.

### 2. Recursive Subdivision in `do_anti_hairline`

The `do_anti_hairline` function uses recursion to subdivide lines longer than 511 pixels. The subdivision limit is small enough that stack depth is unlikely to be an issue. However, for systems with very limited stack space or if the limit were to be increased, this could be converted to an iterative process using a small on-stack queue of line segments to process.

### 3. Per-Pixel Virtual Calls in Blitters

In `Horish_SkAntiHairBlitter::drawLine` and `Vertish_SkAntiHairBlitter::drawLine`, a virtual `blit` call is made for every pixel along the line's major axis. This per-pixel virtual function overhead could be reduced.

A possible optimization would be to change the `SkBlitter` interface to accept a span of pixels to process. For example, `blitAntiVSpan(x, y, count, SkFixed fy, SkFixed dy)` could be a new method. The blitter implementation would then contain the loop, calculating the alpha for each pixel internally. This would amortize the virtual call overhead over the entire span of pixels. This would be a significant change to the blitter API and would affect many other parts of Skia.

### 4. `fastfixdiv` implementation

The `fastfixdiv` function uses integer division, which can be slow on some processors. Since it's only called once per line segment, it's unlikely to be a major bottleneck. However, if profiling shows it to be significant, it could be replaced with a faster alternative, such as a fixed-point multiplication by a reciprocal, if the precision requirements allow for it. Given the inputs are known to be small, a specialized, faster division algorithm could also be used.
