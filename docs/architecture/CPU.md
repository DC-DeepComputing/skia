# Skia CPU Backend Architecture

This document outlines the architecture of Skia's CPU backend. The CPU backend is responsible for rendering graphics on the CPU, without the use of a GPU.

## Background and Definitions

*   **Bitmap**: A grid of pixels representing an image, where each pixel has a color. This is the digital canvas Skia's CPU backend paints on. Common image file formats like PNG and JPEG store bitmap data.
*   **Blend Mode**: A rule that defines how a source color (the one being drawn) is mathematically combined with a destination color (the one already on the canvas).
*   **Blitter**: A component that writes the final pixel colors from the rendering pipeline into the destination bitmap. It's the "hand" that actually puts the colored pixels onto the canvas.
*   **Clip**: A region that restricts drawing. Anything drawn outside the clip is not visible.
*   **Coverage Mask**: An array of values (typically 8-bit alpha) that represents how much a geometric shape covers each pixel. A value of 255 means fully covered, 0 means not covered, and intermediate values represent partial coverage for anti-aliasing.
*   **Hairline**: The thinnest possible line, typically one device pixel wide. A stroke width of 0 in a paint is interpreted as a hairline.
*   **Matrix**: A mathematical tool used to transform (translate, scale, rotate, skew) what is being drawn.
*   **Paint**: An object holding stylistic information for drawing, like color, stroke width, and effects.
*   **Path**: A sequence of lines and curves that describe a shape. Skia uses quadratic and cubic Bézier curves to create smooth, scalable curves. A quadratic curve has one control point, while a cubic curve has two for more complex shapes.
*   **Rasterization**: Converting vector graphics (math-based shapes) into a grid of pixels (a raster image) for display.
*   **Scanline**: A single horizontal row of pixels. Complex shapes are often rendered one scanline at a time.
*   **Shader**: A program that calculates the color of a pixel. Shaders can produce solid colors, gradients, textures, or procedural patterns.
*   **Tiling**: An optimization strategy where a large drawing operation is broken into a series of smaller, tile-sized operations to keep memory usage (e.g., for intermediate buffers) bounded.
*   **Winding Number**: An algorithm used to determine if a point is inside a complex or self-intersecting path. Imagine drawing a ray from the point in any fixed direction to infinity. The winding number is the count of how many times the path crosses the ray in one direction (e.g., clockwise) minus the number of times it crosses in the other direction (counter-clockwise). For a non-zero winding rule, any point with a non-zero winding number is considered "inside" the path. This correctly handles shapes with holes and overlapping sections.

## Architectural Diagram

See [CPU.dot](./CPU.dot) for a diagram of the architecture.

## Architectural Walkthrough

The following sections describe the journey of a single drawing command, like `canvas.drawPath(...)`, through the CPU backend, from the public API call down to the raw memory being changed.

### 1. The API Layer: `SkSurface` and `SkCanvas`

The journey begins with the user-facing API.
- **`SkSurface`**: Represents the entire drawing destination. A CPU-backed surface is typically created with a call like **`SkSurfaces::Raster(...)`**. This factory function allocates a block of memory for the pixels and creates an `SkPixelRef` to own that memory. For the CPU backend, the resulting `SkSurface` object manages an underlying `SkBitmap` that uses this allocation. The surface is the object that creates and provides an `SkCanvas` to draw into it.
- **`SkCanvas`**: This is the primary drawing interface. It exposes methods like `drawPath`, `drawRect`, `drawText`, etc. When a user calls one of these methods, they also provide an **`SkPaint`** object, which describes the styling (color, stroke, effects), and an **`SkPath`** object, which describes the geometry. The `SkCanvas` itself holds the transformation matrix and the clipping region.

When a draw call is made, the `SkCanvas` does not perform the rendering itself. It forwards the command and all its context (path, paint, matrix, clip) to its associated `SkDevice`.

#### Core Paint Components

The `SkPaint` object holds several optional components that control the pixel-processing pipeline.

- **`SkShader`**: The shader is responsible for generating the source color for the geometry being drawn. If a paint does not have a shader, the paint's simple color is used instead. Common types of shaders produce colors from gradients, bitmaps (for drawing images or patterns), or other complex procedural logic.

- **`SkColorFilter`**: After the shader (or the paint's color) produces a source color, the `SkColorFilter` is given a chance to modify it. A color filter is a general-purpose color transformation. Common uses include tinting an image with a specific color or applying a color matrix to perform color correction.

- **`SkMaskFilter`**: The mask filter operates on the alpha channel (or coverage mask) of the shape being drawn, not its color. The most common use for a mask filter is to blur the edges of the shape. For example, `SkMaskFilter::MakeBlur` creates a filter that applies a Gaussian blur to the shape's alpha mask, resulting in a soft, out-of-focus look.


### 2. The Device Layer: `SkBitmapDevice`

For the CPU backend, the `SkCanvas` holds a pointer to an **`SkBitmapDevice`**. This object is the concrete target for all CPU-based drawing. Its main responsibility is to manage the **`SkBitmap`** that is being rendered into.

Upon receiving a draw call from the canvas, the `SkBitmapDevice`'s primary role is to set up and kick off the actual rendering process. It does this by creating and dispatching to an `SkDraw` object.

**A Note on Clipping:** The `SkBitmapDevice` owns an `SkRasterClipStack`, which mirrors the `save()` and `restore()` stack of the `SkCanvas`. When a drawing operation occurs, the device computes the final, effective clip from its stack. This single, resolved clip—represented by an `SkRasterClip` object—is then passed to the `SkDraw` object. `SkDraw` itself is stateless regarding the clip; it only knows about the single clip it was given for its one operation.

**A Note on Tiling:** For large or complex drawing operations, the `SkBitmapDevice` may employ a tiling strategy. Instead of trying to render the entire shape at once, which could require a very large intermediate memory buffer (e.g., for anti-aliasing), the device breaks the draw into a series of smaller, tile-sized operations. It adjusts the clip for each tile and invokes the drawing process multiple times, once for each tile, ensuring that memory usage stays bounded.

### 3. The Orchestrator: `SkDraw`

For each drawing primitive that needs to be rendered, the `SkBitmapDevice` creates an **`SkDraw`** object. `SkDraw` is the orchestrator for a single drawing operation. It bundles together all the necessary context:
- The geometry to be drawn (e.g., the `SkPath`).
- The styling information (the `SkPaint`).
- The transformation and clip from the `SkCanvas`.
- A pointer to the destination `SkPixmap` where pixels will be written.

`SkDraw`'s job is to manage the conversion of the vector-based shape into a series of raster operations. It does this by using `SkScan`.

### 4. The Rasterizer: `SkScan`

`SkDraw` hands the geometric shape (the `SkPath`) to **`SkScan`**. `SkScan` is the core rasterizer. It takes the vector path and converts it into a series of horizontal scanlines. For each scanline, it calculates a coverage mask—an array of alpha values representing how much each pixel on that line is covered by the shape.

`SkScan` does not know anything about colors, shaders, or blend modes. Its only job is to produce these coverage masks. To actually write pixels, `SkDraw` first chooses a `SkBlitter` and then hands that blitter to `SkScan`. As `SkScan` produces each scanline of coverage data, it calls a method on the blitter to render it.

**A Note on Scan Converters:** The `isAntiAlias()` flag on the `SkPaint` determines which of two different scan converter systems is used:
- The **Aliased Scan Converter** (`SkScan_Path.cpp`) is used when anti-aliasing is disabled. It determines if a pixel's center is inside the path. The result is binary: a pixel is either 100% covered or 0% covered. This method results in hard, aliased edges. It calls `blitter->blitH()` to render a solid horizontal run of pixels.
- The **Analytic Anti-Aliased (AAA) Scan Converter** (`SkScan_AAAPath.cpp`) is used when anti-aliasing is enabled. This system identifies critical Y-coordinates on the path (e.g., vertices and intersections). Between these scanlines, it treats the path as a set of simple trapezoids and analytically calculates the exact geometric area of intersection with each pixel. This produces a fractional coverage value for each pixel along the edge of the path. It calls `blitter->blitAntiH()` with an array of alpha values to render the scanline.

### 5. The Pixel Writer: `SkRasterPipelineBlitter`

The **`SkRasterPipelineBlitter`** is the primary type of `SkBlitter` used in the modern CPU backend. It acts as an adapter between the `SkBlitter` interface that `SkScan` expects and the stage-based `SkRasterPipeline`.

When `SkScan` calls a method like `blitAntiH` on the `SkRasterPipelineBlitter` with a coverage mask, the blitter does not write pixels itself. Instead, it runs its internal `SkRasterPipeline`.

### 6. The Workhorse: `SkRasterPipeline`

The **`SkRasterPipeline`** calculates the final color of each pixel. It works by dynamically chaining together a series of single-purpose **stages**.

For example, to draw a path with a solid color, Skia might append stages like `load_a8` (to get pixel coverage), `uniform_color` (to provide the source color), `load_8888_dst` (to get the destination color), `srcover` (to blend), and `store_8888` (to write the result). This stage-based approach allows Skia to support many drawing combinations without a combinatorial explosion of code.

Once the list of stages is finalized, the pipeline is assembled into a sequence of function pointers. This sequence is then executed in a loop. Each stage function is pre-compiled and uses **SIMD (Single Instruction, Multiple Data)** instructions (like SSE or NEON) to process multiple pixels at once. This design allows the pipeline to be constructed dynamically at runtime while maintaining high performance.

To perform its final step (loading and storing pixel values), the pipeline needs to know the memory layout of the destination bitmap. It gets this information from a `SkPixmap`.

### 7. The Pixel Memory View: `SkPixmap`, `SkBitmap`, and `SkPixelRef`

This is the final stop, where pixels are actually modified.
- **`SkPixmap`**: A lightweight object that encapsulates a pointer to a block of pixel memory along with its metadata (width, height, color type, row bytes). The `SkRasterPipeline` uses the `SkPixmap` to get the exact address of the pixels it needs to read from and write to.
- **`SkBitmap`**: The `SkBitmapDevice` holds an `SkBitmap`. The bitmap is an object that pairs an `SkImageInfo` with the actual pixel storage.
- **`SkPixelRef`**: The pixel storage itself is managed by a smart pointer called **`SkPixelRef`**. This object is the owner of the heap-allocated memory for the pixels, which was created when the `SkSurface` was first made.

When the `SkRasterPipeline`'s final stage runs, it uses the address provided by the `SkPixmap` to write a new color value into the block of memory owned by the `SkPixelRef`. At this point, the drawing command is complete, and the image has been visibly changed.