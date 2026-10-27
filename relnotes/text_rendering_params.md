`SkGraphics::SetTextRenderingParams` configures process-wide text gamma,
contrast, A8 gamma correction, and the LCD filter used for path-generated
glyph masks. Call it once during startup, before creating surfaces, strikes,
or GPU contexts, and never concurrently with other Skia calls. Defaults
preserve the existing build and font-backend policies. The LCD filter choice
does not affect native font-backend bitmaps.
