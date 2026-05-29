# Warnings Fixed

## Phase 1: modernize-make-unique

- **include/core/SkStream.h:322**: Replaced `std::unique_ptr<SkFILEStream>(new SkFILEStream(path))` with `std::make_unique<SkFILEStream>(path)`.
- **src/codec/SkJpegCodec.cpp:85**: Replaced `std::unique_ptr<JpegDecoderMgr>(new JpegDecoderMgr(stream))` with `std::make_unique<JpegDecoderMgr>(stream)`.
- **src/codec/SkXmp.cpp:653, 661**: Replaced `std::unique_ptr<SkXmpImpl>(new SkXmpImpl)` with `std::make_unique<SkXmpImpl>()`.
- **src/gpu/ganesh/ops/LatticeOp.cpp:574**: Replaced `std::unique_ptr<SkLatticeIter>(new SkLatticeIter(lattice, dst))` with `std::make_unique<SkLatticeIter>(lattice, dst)`.
- **src/pdf/SkPDFTypes.h:144**: Replaced `std::unique_ptr<SkPDFArray>(new SkPDFArray())` with `std::make_unique<SkPDFArray>()`.
- **src/ports/SkFontHost_FreeType.cpp:582**: Replaced `std::unique_ptr<SkAdvancedTypefaceMetrics>(new SkAdvancedTypefaceMetrics)` with `std::make_unique<SkAdvancedTypefaceMetrics>()`.
- **src/ports/SkFontHost_FreeType.cpp:730**: Replaced `std::unique_ptr<SkScalerContext_FreeType>(new SkScalerContext_FreeType(...))` with `std::make_unique<SkScalerContext_FreeType>(...)`.
- **src/ports/SkFontMgr_android_parser.cpp:266, 414**: Replaced `std::unique_ptr<FontFamily>(new FontFamily(...))` with `std::make_unique<FontFamily>(...)`.
- **src/utils/SkCanvasStateUtils.cpp:186, 291, 310**: Replaced manual `std::unique_ptr` creation with `std::make_unique` for `SkCanvasState_v1`, `SkCanvas`, and `SkCanvasStack`.
- **tools/fonts/TestSVGTypeface.cpp:149**: Replaced `std::unique_ptr<SkAdvancedTypefaceMetrics>(new SkAdvancedTypefaceMetrics)` with `std::make_unique<SkAdvancedTypefaceMetrics>()`.
- **tools/fonts/TestTypeface.cpp:158**: Replaced `std::unique_ptr<SkAdvancedTypefaceMetrics>(new SkAdvancedTypefaceMetrics)` with `std::make_unique<SkAdvancedTypefaceMetrics>()`.
  - **Justification:** Follows modern C++ best practices, is more concise, and ensures exception safety.
