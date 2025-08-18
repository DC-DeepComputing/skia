# Analysis for Combining SkDraw-related Headers

This document outlines the analysis and plan for consolidating `src/core/SkGlyphRunPainter.h`, `src/core/SkDrawBase.h`, and `src/core/SkDraw.h` into a single file. The goal is to improve code cohesion and make the `SkDraw` class easier to understand and maintain.

## 1. Current State Analysis

The functionality related to `SkDraw` is currently spread across three header files, creating a complex inheritance structure.

### File Breakdown

*   **`src/core/SkGlyphRunPainter.h`**: Defines `SkGlyphRunListPainterCPU` and its nested abstract class `BitmapDevicePainter`. `BitmapDevicePainter` serves as an interface for painting glyphs and bitmaps, and is used by `SkGlyphRunListPainterCPU` to abstract the destination device.
*   **`src/core/SkDrawBase.h`**: Defines `SkDrawBase`, which inherits from `BitmapDevicePainter`. It provides a large set of common drawing methods. It implements the `BitmapDevicePainter` interface with "not supported" stubs, essentially making it an abstract base class in practice for glyph painting.
*   **`src/core/SkDraw.h`**: Defines `SkDraw`, which inherits from `SkDrawBase`. This is the primary, concrete implementation used for CPU-based drawing. It overrides the `BitmapDevicePainter` methods with functional implementations and adds further drawing capabilities.

### Class Hierarchy

The class hierarchy is as follows:

```
SkGlyphRunListPainterCPU::BitmapDevicePainter (interface)
└── SkDrawBase (base implementations, stub interface methods)
    └── SkDraw (concrete implementation)
```

### Problem Statement

This separation of concerns across three files and a three-level inheritance hierarchy is detrimental to code comprehension. Given that `SkDraw` is the sole concrete implementation of this entire stack, the structure is unnecessarily fragmented. A developer seeking to understand `SkDraw` must navigate this hierarchy and multiple files to get a complete picture.

## 2. Investigation of Further Simplification

An investigation was conducted to see if `SkGlyphRunListPainterCPU` and its `BitmapDevicePainter` interface could also be merged into `SkDraw`.

The analysis revealed that `SkOverdrawCanvas` uses a custom implementation of the `BitmapDevicePainter` interface (`SkOverdrawCanvas::TextDevice`). This means the abstraction is necessary and cannot be removed. `SkGlyphRunListPainterCPU` must continue to operate on the `BitmapDevicePainter` interface to support both `SkDraw` and `SkOverdrawCanvas::TextDevice`.

## 3. Revised Consolidation Plan

The proposal is to merge `SkDrawBase` into `SkDraw`, move the result into the `skcpu` namespace, and relocate the glyph painting classes to a more appropriate home.

### Step-by-step Plan

1.  **Consolidate `SkDraw` and `SkDrawBase`**:
    *   Create new files: `src/core/skcpu/SkDraw.h` and `src/core/skcpu/SkDraw.cpp`.
    *   Define a new class `skcpu::Draw` in the new header.
    *   Merge the contents of `SkDrawBase` and `SkDraw` into `skcpu::Draw`.
    *   `skcpu::Draw` will directly inherit from `SkGlyphRunListPainterCPU::BitmapDevicePainter`.
    *   All members from `SkDrawBase` and `SkDraw` will be moved into `skcpu::Draw`. The stub implementations from `SkDrawBase` will be discarded.
    *   The implementation code from `SkDrawBase.cpp` and `SkDraw.cpp` will be moved into `src/core/skcpu/SkDraw.cpp`.

2.  **Relocate Glyph Painter**:
    *   Move `src/core/SkGlyphRunPainter.h` to `src/core/skcpu/SkGlyphRunPainter.h`.
    *   Move `src/core/SkGlyphRunPainter.cpp` to `src/core/skcpu/SkGlyphRunPainter.cpp`.
    *   Update the include guards and paths accordingly.

3.  **Update Codebase**:
    *   Search the codebase for any `#include` directives for the old headers (`SkDraw.h`, `SkDrawBase.h`, `SkGlyphRunPainter.h`) and update them to point to the new files (`skcpu/SkDraw.h`, `skcpu/SkGlyphRunPainter.h`).
    *   Replace usages of `SkDraw` with `skcpu::Draw`.

4.  **Update Build System**:
    *   Remove all references to the old files from the build system (`BUILD.gn`, `BUILD.bazel`).
    *   Add the new files (`skcpu/SkDraw.h`, `skcpu/SkDraw.cpp`, `skcpu/SkGlyphRunPainter.h`, `skcpu/SkGlyphRunPainter.cpp`) to the build system.

## 4. Resulting Structure

*   **`src/core/skcpu/SkGlyphRunPainter.h`**: Contains `SkGlyphRunListPainterCPU` and the `BitmapDevicePainter` interface.
*   **`src/core/skcpu/SkDraw.h`**: Contains the new, consolidated `skcpu::Draw` class.
*   **Class Hierarchy**:
    ```
    SkGlyphRunListPainterCPU::BitmapDevicePainter (interface)
    ├── skcpu::Draw (concrete implementation)
    └── SkOverdrawCanvas::TextDevice (concrete implementation)
    ```

## 5. Justification and Benefits

This refactoring offers several advantages:

*   **Improved Cohesion and Clarity**: The main CPU drawing logic is consolidated into a single, clearly-named class (`skcpu::Draw`).
*   **Reduced Complexity**: Flattening the inheritance hierarchy of `SkDraw` removes unnecessary layers of abstraction.
*   **Better Organization**: Moving CPU-specific drawing and text painting code into the `skcpu` directory improves the overall organization of the codebase.
*   **Maintained Abstraction**: The necessary `BitmapDevicePainter` interface is preserved, allowing `SkOverdrawCanvas` to continue functioning correctly.