# DO NOT EDIT: This file is auto-generated.
cmake_minimum_required(VERSION 3.13)

project(Skia LANGUAGES C CXX)

set(DEFAULT_COMPILE_FLAGS_MACOS "-std=c++17 -Wno-psabi --target=arm64-apple-macos11")
set(DEFAULT_COMPILE_FLAGS_LINUX "-std=c++17 -Wno-psabi -Wno-attributes")

if (APPLE)
  set(DEFAULT_COMPILE_FLAGS "${DEFAULT_COMPILE_FLAGS_MACOS}")
else()
  set(DEFAULT_COMPILE_FLAGS "${DEFAULT_COMPILE_FLAGS_LINUX}")
endif()


# //include/android:public_hdrs
list(APPEND include_android_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/android/SkAndroidFrameworkUtils.h"
    "${CMAKE_SOURCE_DIR}/include/android/SkAnimatedImage.h"
)

# //include/codec:public_hdrs
list(APPEND include_codec_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/codec/SkAndroidCodec.h"
    "${CMAKE_SOURCE_DIR}/include/codec/SkCodec.h"
    "${CMAKE_SOURCE_DIR}/include/codec/SkCodecAnimation.h"
    "${CMAKE_SOURCE_DIR}/include/codec/SkEncodedOrigin.h"
)

# //include/config:public_hdrs
list(APPEND include_config_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/config/SkUserConfig.h"
)

# //include/core:public_hdrs
list(APPEND include_core_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/core/SkAlphaType.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkAnnotation.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkBBHFactory.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkBitmap.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkBlendMode.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkBlender.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkBlurTypes.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkCanvas.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkCanvasVirtualEnforcer.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkCapabilities.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkClipOp.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkColor.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkColorFilter.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkColorPriv.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkColorSpace.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkColorType.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkContourMeasure.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkCoverageMode.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkCubicMap.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkData.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkDataTable.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkDeferredDisplayList.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkDeferredDisplayListRecorder.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkDocument.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkDrawLooper.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkDrawable.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkEncodedImageFormat.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkExecutor.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFlattenable.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFont.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFontArguments.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFontMetrics.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFontMgr.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFontParameters.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFontStyle.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkFontTypes.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkGraphics.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkICC.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkImage.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkImageEncoder.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkImageFilter.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkImageGenerator.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkImageInfo.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkM44.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkMallocPixelRef.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkMaskFilter.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkMath.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkMatrix.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkMesh.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkMilestone.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkOpenTypeSVGDecoder.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkOverdrawCanvas.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPaint.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPath.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPathBuilder.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPathMeasure.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPathTypes.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPicture.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPictureRecorder.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPixelRef.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPixmap.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPngChunkReader.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPoint.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPoint3.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkPromiseImageTexture.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkRRect.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkRSXform.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkRasterHandleAllocator.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkRect.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkRefCnt.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkRegion.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSamplingOptions.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkScalar.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSerialProcs.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkShader.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSize.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSpan.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkStream.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkString.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkStrokeRec.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSurface.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSurfaceCharacterization.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSurfaceProps.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkSwizzle.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkTextBlob.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkTileMode.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkTime.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkTraceMemoryDump.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkTypeface.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkTypes.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkUnPreMultiply.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkVertices.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkYUVAInfo.h"
    "${CMAKE_SOURCE_DIR}/include/core/SkYUVAPixmaps.h"
)

# //include/effects:public_hdrs
list(APPEND include_effects_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/effects/Sk1DPathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/Sk2DPathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkBlenders.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkBlurDrawLooper.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkBlurMaskFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkColorMatrix.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkColorMatrixFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkCornerPathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkDashPathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkDiscretePathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkGradientShader.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkHighContrastFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkImageFilters.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkLayerDrawLooper.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkLumaColorFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkOpPathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkOverdrawColorFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkPerlinNoiseShader.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkRuntimeEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkShaderMaskFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkStrokeAndFillPathEffect.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkTableColorFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkTableMaskFilter.h"
    "${CMAKE_SOURCE_DIR}/include/effects/SkTrimPathEffect.h"
)

# //include/encode:public_hdrs
list(APPEND include_encode_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/encode/SkEncoder.h"
    "${CMAKE_SOURCE_DIR}/include/encode/SkJpegEncoder.h"
    "${CMAKE_SOURCE_DIR}/include/encode/SkPngEncoder.h"
    "${CMAKE_SOURCE_DIR}/include/encode/SkWebpEncoder.h"
)

# //include/pathops:public_hdrs
list(APPEND include_pathops_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/pathops/SkPathOps.h"
)

# //include/ports:public_hdrs
list(APPEND include_ports_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/ports/SkCFObject.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontConfigInterface.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_FontConfigInterface.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_android.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_directory.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_empty.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_fontconfig.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_fuchsia.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_indirect.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkFontMgr_mac_ct.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkImageGeneratorCG.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkImageGeneratorNDK.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkImageGeneratorWIC.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkRemotableFontMgr.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkTypeface_mac.h"
    "${CMAKE_SOURCE_DIR}/include/ports/SkTypeface_win.h"
)

# //include/utils:public_hdrs
list(APPEND include_utils_public_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/utils/SkAnimCodecPlayer.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkBase64.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkCamera.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkCanvasStateUtils.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkCustomTypeface.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkEventTracer.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkNWayCanvas.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkNoDrawCanvas.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkNullCanvas.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkOrderedFontMgr.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkPaintFilterCanvas.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkParse.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkParsePath.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkRandom.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkShadowUtils.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkTextUtils.h"
    "${CMAKE_SOURCE_DIR}/include/utils/SkTraceEventPhase.h"
)

# //include:public_hdrs
list(APPEND include_public_hdrs
    # Sources:
    ${include_android_public_hdrs}
    ${include_codec_public_hdrs}
    ${include_config_public_hdrs}
    ${include_core_public_hdrs}
    ${include_docs_public_hdrs}
    ${include_effects_public_hdrs}
    ${include_encode_public_hdrs}
    ${include_pathops_public_hdrs}
    ${include_ports_public_hdrs}
    ${include_utils_public_hdrs}
)

# //include/private/chromium:private_hdrs
list(APPEND include_private_chromium_private_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/private/chromium/SkChromeRemoteGlyphCache.h"
    "${CMAKE_SOURCE_DIR}/include/private/chromium/Slug.h"
)

# //include/private:private_hdrs
list(APPEND include_private_private_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/include/private/SingleOwner.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkBitmaskEnum.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkChecksum.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkColorData.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkDeque.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkEncodedInfo.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkFixed.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkFloatBits.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkFloatingPoint.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkHalf.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkIDChangeListener.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkImageInfoPriv.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkMacros.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkMalloc.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkMutex.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkNoncopyable.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkOnce.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkOpts_spi.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkPathRef.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLDefines.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLIRNode.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLLayout.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLModifiers.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLProgramElement.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLProgramKind.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLSampleUsage.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLStatement.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLString.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSLSymbol.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSafe32.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSafe_math.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSemaphore.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkShadowFlags.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkSpinlock.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkStringView.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTArray.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTDArray.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTFitsIn.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTHash.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTLogic.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTPin.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTemplates.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkThreadAnnotations.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkThreadID.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkTo.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkUniquePaintParamsID.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkVx.h"
    "${CMAKE_SOURCE_DIR}/include/private/SkWeakRefCnt.h"
    ${include_private_chromium_private_hdrs}
)

# //include:private_hdrs
list(APPEND include_private_hdrs
    # Sources:
    ${include_private_private_hdrs}
)

# //src/codec:decode_android_hdrs
list(APPEND src_codec_decode_android_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/codec/SkAndroidCodecAdapter.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkSampledCodec.h"
)

# //src/codec:decode_bmp_hdrs
list(APPEND src_codec_decode_bmp_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpBaseCodec.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpCodec.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpMaskCodec.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpRLECodec.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpStandardCodec.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkWbmpCodec.h"
)

# //src/codec:core_hdrs
list(APPEND src_codec_core_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/codec/SkCodecImageGenerator.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkCodecPriv.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkColorTable.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkFrameHolder.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkMaskSwizzler.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkMasks.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkSampler.h"
    "${CMAKE_SOURCE_DIR}/src/codec/SkSwizzler.h"
)

# //src/codec:private_hdrs
list(APPEND src_codec_private_hdrs
    # Sources:
    ${src_codec_decode_android_hdrs}
    ${src_codec_decode_bmp_hdrs}
    ${src_codec_core_hdrs}
)

# //src/core:core_hdrs
list(APPEND src_core_core_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/core/Sk4px.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAAClip.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkASAN.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkATrace.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAdvancedTypefaceMetrics.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAnalyticEdge.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAnnotationKeys.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAntiRun.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkArenaAlloc.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkArenaAllocList.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAutoBlitterChoose.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAutoMalloc.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkAutoPixmapStorage.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBigPicture.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmapCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmapDevice.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmapProcState.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlendModeBlender.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlendModePriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlenderBase.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitBWMaskTemplate.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitRow.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlockAllocator.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlurMask.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBuffer.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkBuiltInCodeSnippetID.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkCachedData.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkCanvasPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkClipStack.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkClipStackDevice.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorFilterBase.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorFilterPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorFilter_Matrix.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorSpacePriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorSpaceXformSteps.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkCompressedDataUtils.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkConvertPixels.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkCoreBlitters.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkCpu.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkCubicClipper.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkCubicSolver.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDebugUtils.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDeferredDisplayListPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDescriptor.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDevice.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDiscardableMemory.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDistanceFieldGen.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDraw.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDrawProcs.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkDrawShadowInfo.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkEdge.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkEdgeBuilder.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkEdgeClipper.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkEffectPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkEndian.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkEnumBitMask.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkEnumerate.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkFDot6.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkFontDescriptor.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkFontPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkFontStream.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkFuzzLogging.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkGaussFilter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkGeometry.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkGlyph.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkGlyphBuffer.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkGlyphRunPainter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkGpuBlurUtils.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkICCPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkIPoint16.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageFilterCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageFilterTypes.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageFilter_Base.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkImagePriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkLRUCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkLatticeIter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkLeanWindows.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkLineClipper.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkLocalMatrixImageFilter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMD5.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMSAN.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMask.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskBlurFilter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskFilterBase.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskGamma.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMathPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrixImageFilter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrixInvert.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrixPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrixProvider.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrixUtils.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMeshPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMessageBus.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMiniRecorder.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMipmap.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMipmapAccessor.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkMipmapBuilder.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkModeColorFilter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkNextID.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkOSFile.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkOpts.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPaintDefaults.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPaintPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathEffectBase.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathMakers.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathMeasurePriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureCommon.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureData.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureFlat.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPicturePlayback.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPicturePriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureRecord.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPipelineData.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPixelRefPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPixmapPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPointPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkPtrRecorder.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkQuadClipper.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRRectPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRTree.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRasterClip.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRasterClipStack.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRasterPipeline.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkReadBuffer.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecord.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecordDraw.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecordOpts.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecordPattern.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecordedDrawable.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecorder.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecords.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRectPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRegionPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkResourceCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRuntimeEffectDictionary.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkRuntimeEffectPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSafeMath.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSafeRange.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSamplingPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkScaleToSides.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkScalerCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkScalerContext.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkScan.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkScanPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkScopeExit.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSharedMutex.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpecialImage.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpecialSurface.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpriteBlitter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkStreamPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkStrikeCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkStrikeSpec.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkStringUtils.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkStroke.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkStrokerPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkSurfacePriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTBlockList.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTDPQueue.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTDynamicHash.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTInternalLList.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTLazy.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTMultiMap.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTSearch.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTSort.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTaskGroup.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTextBlobPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTextBlobTrace.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTextFormatParams.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTraceEvent.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTraceEventCommon.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTypefaceCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkTypeface_remote.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkUtils.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkVM.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkVMBlitter.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkVM_fwd.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkValidationUtils.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkVertState.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkVerticesPriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkWriteBuffer.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkWritePixelsRec.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkWriter32.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkXfermodeInterpretation.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkXfermodePriv.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkYUVAInfoLocation.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkYUVMath.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkYUVPlanesCache.h"
    "${CMAKE_SOURCE_DIR}/src/core/SkZip.h"
)

# //src/core:private_hdrs
list(APPEND src_core_private_hdrs
    # Sources:
    ${src_core_core_hdrs}
)

# //src/effects:effects_hdrs
list(APPEND src_effects_effects_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/effects/SkDashImpl.h"
    "${CMAKE_SOURCE_DIR}/src/effects/SkEmbossMask.h"
    "${CMAKE_SOURCE_DIR}/src/effects/SkEmbossMaskFilter.h"
    "${CMAKE_SOURCE_DIR}/src/effects/SkOpPE.h"
    "${CMAKE_SOURCE_DIR}/src/effects/SkTrimPE.h"
)

# //src/effects/imagefilters:imagefilters_hdrs
list(APPEND src_effects_imagefilters_imagefilters_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkCropImageFilter.h"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkRuntimeImageFilter.h"
)

# //src/effects/imagefilters:private_hdrs
list(APPEND src_effects_imagefilters_private_hdrs
    # Sources:
    ${src_effects_imagefilters_imagefilters_hdrs}
)

# //src/effects:private_hdrs
list(APPEND src_effects_private_hdrs
    # Sources:
    ${src_effects_effects_hdrs}
    ${src_effects_imagefilters_private_hdrs}
)

# //src/image:core_hdrs
list(APPEND src_image_core_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/image/SkImage_Base.h"
    "${CMAKE_SOURCE_DIR}/src/image/SkImage_Lazy.h"
    "${CMAKE_SOURCE_DIR}/src/image/SkReadPixelsRec.h"
    "${CMAKE_SOURCE_DIR}/src/image/SkRescaleAndReadPixels.h"
    "${CMAKE_SOURCE_DIR}/src/image/SkSurface_Base.h"
)

# //src/image:private_hdrs
list(APPEND src_image_private_hdrs
    # Sources:
    ${src_image_core_hdrs}
)

# //src/images:private_hdrs
list(APPEND src_images_private_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/images/SkImageEncoderFns.h"
    "${CMAKE_SOURCE_DIR}/src/images/SkImageEncoderPriv.h"
)

# //src/opts:private_hdrs
list(APPEND src_opts_private_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/opts/SkBitmapProcState_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkBlitMask_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkBlitRow_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkChecksum_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkRasterPipeline_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkSwizzler_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkUtils_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkVM_opts.h"
    "${CMAKE_SOURCE_DIR}/src/opts/SkXfermode_opts.h"
)

# //src/pathops:pathops_hdrs
list(APPEND src_pathops_pathops_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/pathops/SkAddIntersections.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkIntersectionHelper.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkIntersections.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkLineParameters.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpAngle.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpCoincidence.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpContour.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpEdgeBuilder.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpSegment.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpSpan.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsBounds.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsCommon.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsConic.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsCubic.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsCurve.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsDebug.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsLine.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsPoint.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsQuad.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsRect.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsTCurve.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsTSect.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsTypes.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathWriter.h"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkReduceOrder.h"
)

# //src/pathops:private_hdrs
list(APPEND src_pathops_private_hdrs
    # Sources:
    ${src_pathops_pathops_hdrs}
)

# //src/ports:private_hdrs
list(APPEND src_ports_private_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkOSLibrary.h"
)

# //src/sfnt:sfnt_hdrs
list(APPEND src_sfnt_sfnt_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkIBMFamilyClass.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTableTypes.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_EBDT.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_EBLC.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_EBSC.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_OS_2.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_OS_2_V0.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_OS_2_V1.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_OS_2_V2.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_OS_2_V3.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_OS_2_V4.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_OS_2_VA.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_fvar.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_gasp.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_glyf.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_head.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_hhea.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_hmtx.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_loca.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_maxp.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_maxp_CFF.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_maxp_TT.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_name.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_post.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTUtils.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkPanose.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkSFNTHeader.h"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkTTCFHeader.h"
)

# //src/sfnt:private_hdrs
list(APPEND src_sfnt_private_hdrs
    # Sources:
    ${src_sfnt_sfnt_hdrs}
)

# //src/shaders:shader_hdrs
list(APPEND src_shaders_shader_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/shaders/SkBitmapProcShader.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkColorFilterShader.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkImageShader.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkLocalMatrixShader.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkPictureShader.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkShaderBase.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkTransformShader.h"
)

# //src/shaders/gradients:gradient_hdrs
list(APPEND src_shaders_gradients_gradient_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/Sk4fGradientBase.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/Sk4fGradientPriv.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/Sk4fLinearGradient.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkGradientShaderBase.h"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkLinearGradient.h"
)

# //src/shaders/gradients:private_hdrs
list(APPEND src_shaders_gradients_private_hdrs
    # Sources:
    ${src_shaders_gradients_gradient_hdrs}
)

# //src/shaders:private_hdrs
list(APPEND src_shaders_private_hdrs
    # Sources:
    ${src_shaders_shader_hdrs}
    ${src_shaders_gradients_private_hdrs}
)

# //src/text:text_hdrs
list(APPEND src_text_text_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/text/GlyphRun.h"
    "${CMAKE_SOURCE_DIR}/src/text/StrikeForGPU.h"
)

# //src/text:private_hdrs
list(APPEND src_text_private_hdrs
    # Sources:
    ${src_text_text_hdrs}
)

# //src/utils:core_hdrs
list(APPEND src_utils_core_hdrs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/utils/SkBitSet.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkBlitterTrace.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkBlitterTraceCommon.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCallableTraits.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCanvasStack.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCharToGlyphCache.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkClipStackUtils.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCycles.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkDashPathPriv.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkFloatToDecimal.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkFloatUtils.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkJSON.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkJSONWriter.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkMatrix22.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkMultiPictureDocument.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkMultiPictureDocumentPriv.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkOSPath.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkPatchUtils.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkPolyUtils.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkShadowTessellator.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkTestCanvas.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkUTF.h"
    "${CMAKE_SOURCE_DIR}/src/utils/SkVMVisualizer.h"
)

# //src/utils:private_hdrs
list(APPEND src_utils_private_hdrs
    # Sources:
    ${src_utils_core_hdrs}
)

# //src:private_hdrs
list(APPEND src_private_hdrs
    # Sources:
    ${src_codec_private_hdrs}
    ${src_core_private_hdrs}
    ${src_effects_private_hdrs}
    ${src_image_private_hdrs}
    ${src_images_private_hdrs}
    ${src_opts_private_hdrs}
    ${src_pathops_private_hdrs}
    ${src_ports_private_hdrs}
    ${src_sfnt_private_hdrs}
    ${src_shaders_private_hdrs}
    ${src_text_private_hdrs}
    ${src_utils_private_hdrs}
)

# //src/android:srcs
list(APPEND src_android_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/android/SkAndroidFrameworkUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/android/SkAnimatedImage.cpp"
)

# //src/codec:decode_android_srcs
list(APPEND src_codec_decode_android_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/codec/SkAndroidCodec.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkAndroidCodecAdapter.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkSampledCodec.cpp"
)

# //src/codec:decode_bmp_srcs
list(APPEND src_codec_decode_bmp_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpBaseCodec.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpCodec.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpMaskCodec.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpRLECodec.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkBmpStandardCodec.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkWbmpCodec.cpp"
)

# //src/codec:core_srcs
list(APPEND src_codec_core_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/codec/SkCodec.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkCodecImageGenerator.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkColorTable.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkMaskSwizzler.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkMasks.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkSampler.cpp"
    "${CMAKE_SOURCE_DIR}/src/codec/SkSwizzler.cpp"
)

# //src/codec:srcs
list(APPEND src_codec_srcs
    # Sources:
    ${src_codec_decode_android_srcs}
    ${src_codec_decode_bmp_srcs}
    ${src_codec_core_srcs}
)

# //src/core:core_srcs
list(APPEND src_core_core_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/core/SkAAClip.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkATrace.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkAlphaRuns.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkAnalyticEdge.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkAnnotation.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkArenaAlloc.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkAutoPixmapStorage.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBBHFactory.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBigPicture.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmap.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmapCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmapDevice.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmapProcState.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBitmapProcState_matrixProcs.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlendMode.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlendModeBlender.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitRow_D32.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitter_A8.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitter_ARGB32.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitter_RGB565.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlitter_Sprite.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlockAllocator.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlurMF.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBlurMask.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkBuffer.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCachedData.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCanvas.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCanvasPriv.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCapabilities.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkChromeRemoteGlyphCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkClipStack.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkClipStackDevice.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkColor.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorFilter_Matrix.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorSpace.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkColorSpaceXformSteps.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCompressedDataUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkContourMeasure.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkConvertPixels.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCpu.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCubicClipper.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkCubicMap.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkData.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDataTable.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDebug.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDeferredDisplayList.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDeferredDisplayListRecorder.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDeque.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDescriptor.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDevice.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDistanceFieldGen.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDocument.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDraw.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDrawLooper.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDrawShadowInfo.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDraw_atlas.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDraw_text.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDraw_vertices.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkDrawable.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkEdge.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkEdgeBuilder.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkEdgeClipper.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkExecutor.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkFlattenable.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkFont.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkFontDescriptor.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkFontMgr.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkFontStream.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkFont_serial.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGaussFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGeometry.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGlobalInitialization_core.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGlyph.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGlyphBuffer.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGlyphRunPainter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGpuBlurUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkGraphics.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkHalf.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkICC.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkIDChangeListener.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageFilterCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageFilterTypes.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageGenerator.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkImageInfo.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkLatticeIter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkLineClipper.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkLocalMatrixImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkM44.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMD5.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMalloc.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMallocPixelRef.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMask.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskBlurFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMaskGamma.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMath.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrix.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrixImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMatrixInvert.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMesh.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMiniRecorder.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMipmap.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkMipmapAccessor.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkModeColorFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkOpts.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkOpts_erms.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkOverdrawCanvas.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPaint.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPaintPriv.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPath.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathBuilder.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathMeasure.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPathRef.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPath_serial.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPicture.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureData.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureFlat.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureImageGenerator.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPicturePlayback.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureRecord.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPictureRecorder.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPixelRef.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPixmap.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPoint.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPoint3.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPromiseImageTexture.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkPtrRecorder.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkQuadClipper.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRRect.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRTree.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRasterClip.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRasterPipeline.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRasterPipelineBlitter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkReadBuffer.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecord.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecordDraw.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecordOpts.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecordedDrawable.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecorder.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRecords.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRect.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRegion.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkRegion_path.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkResourceCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScalar.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScalerCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScalerContext.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScan.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScan_AAAPath.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScan_AntiPath.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScan_Antihair.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScan_Hairline.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkScan_Path.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSemaphore.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSharedMutex.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpecialImage.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpecialSurface.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpinlock.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpriteBlitter_ARGB32.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSpriteBlitter_RGB565.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkStream.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkStrikeCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkStrikeSpec.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkString.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkStringUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkStroke.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkStrokeRec.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkStrokerPriv.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSurfaceCharacterization.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkSwizzle.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTSearch.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTaskGroup.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTextBlob.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTextBlobTrace.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkThreadID.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTime.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTypeface.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTypefaceCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkTypeface_remote.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkUnPreMultiply.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkUtilsArm.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkVM.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkVMBlitter.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkVertState.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkVertices.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkWriteBuffer.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkWriter32.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkXfermode.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkXfermodeInterpretation.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkYUVAInfo.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkYUVAPixmaps.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkYUVMath.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/SkYUVPlanesCache.cpp"
)

# //src/core:srcs
list(APPEND src_core_srcs
    # Sources:
    ${src_core_core_srcs}
)

# //src/effects:effects_srcs
list(APPEND src_effects_effects_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/effects/Sk1DPathEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/Sk2DPathEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkBlenders.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkColorMatrix.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkColorMatrixFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkCornerPathEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkDashPathEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkDiscretePathEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkEmbossMask.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkEmbossMaskFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkHighContrastFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkLayerDrawLooper.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkLumaColorFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkOpPathEffect.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkOverdrawColorFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkShaderMaskFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkTableColorFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkTableMaskFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/SkTrimPathEffect.cpp"
)

# //src/effects/imagefilters:imagefilters_srcs
list(APPEND src_effects_imagefilters_imagefilters_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkAlphaThresholdImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkArithmeticImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkBlendImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkBlurImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkColorFilterImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkComposeImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkCropImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkDisplacementMapImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkDropShadowImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkImageImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkLightingImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkMagnifierImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkMatrixConvolutionImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkMergeImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkMorphologyImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkOffsetImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkPictureImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkRuntimeImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkShaderImageFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/effects/imagefilters/SkTileImageFilter.cpp"
)

# //src/effects/imagefilters:srcs
list(APPEND src_effects_imagefilters_srcs
    # Sources:
    ${src_effects_imagefilters_imagefilters_srcs}
)

# //src/effects:srcs
list(APPEND src_effects_srcs
    # Sources:
    ${src_effects_effects_srcs}
    ${src_effects_imagefilters_srcs}
)

# //src/image:core_srcs
list(APPEND src_image_core_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/image/SkImage.cpp"
    "${CMAKE_SOURCE_DIR}/src/image/SkImage_Lazy.cpp"
    "${CMAKE_SOURCE_DIR}/src/image/SkImage_Raster.cpp"
    "${CMAKE_SOURCE_DIR}/src/image/SkRescaleAndReadPixels.cpp"
    "${CMAKE_SOURCE_DIR}/src/image/SkSurface.cpp"
    "${CMAKE_SOURCE_DIR}/src/image/SkSurface_Raster.cpp"
)

# //src/image:srcs
list(APPEND src_image_srcs
    # Sources:
    ${src_image_core_srcs}
)

# //src/images:srcs
list(APPEND src_images_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/images/SkImageEncoder.cpp"
)

# //src/opts:srcs
list(APPEND src_opts_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/opts/SkOpts_crc32.cpp"
)

# //src/pathops:pathops_srcs
list(APPEND src_pathops_pathops_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/pathops/SkAddIntersections.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkDConicLineIntersection.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkDCubicLineIntersection.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkDCubicToQuads.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkDLineIntersection.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkDQuadLineIntersection.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkIntersections.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpAngle.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpBuilder.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpCoincidence.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpContour.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpCubicHull.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpEdgeBuilder.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpSegment.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkOpSpan.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsAsWinding.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsCommon.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsConic.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsCubic.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsCurve.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsDebug.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsLine.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsOp.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsQuad.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsRect.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsSimplify.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsTSect.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsTightBounds.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsTypes.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathOpsWinding.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkPathWriter.cpp"
    "${CMAKE_SOURCE_DIR}/src/pathops/SkReduceOrder.cpp"
)

# //src/pathops:srcs
list(APPEND src_pathops_srcs
    # Sources:
    ${src_pathops_pathops_srcs}
)

# //src/ports:fontmgr_empty_factory
list(APPEND src_ports_fontmgr_empty_factory
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkFontMgr_empty_factory.cpp"
)

# //src/ports:fontmgr_factory
list(APPEND src_ports_fontmgr_factory
    # Sources:
    ${src_ports_fontmgr_empty_factory}
)

# //src/ports:global_init
list(APPEND src_ports_global_init
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkGlobalInitialization_default.cpp"
)

# //src/ports:image_generator
list(APPEND src_ports_image_generator
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkImageGenerator_skia.cpp"
)

# //src/ports:load_dynamic_library
list(APPEND src_ports_load_dynamic_library
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkOSLibrary_posix.cpp"
)

# //src/ports:malloc
list(APPEND src_ports_malloc
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkMemory_malloc.cpp"
)

# //src/ports:osfile
list(APPEND src_ports_osfile
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkOSFile_stdio.cpp"
    "${CMAKE_SOURCE_DIR}/src/ports/SkOSFile_posix.cpp"
)

# //src/ports:skdebug
list(APPEND src_ports_skdebug
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/ports/SkDebug_stdio.cpp"
)

# //src/ports:srcs
list(APPEND src_ports_srcs
    # Sources:
    ${src_ports_fontmgr}
    ${src_ports_fontmgr_factory}
    ${src_ports_global_init}
    ${src_ports_image_generator}
    ${src_ports_load_dynamic_library}
    ${src_ports_malloc}
    ${src_ports_osfile}
    ${src_ports_skdebug}
)

# //src/sfnt:sfnt_srcs
list(APPEND src_sfnt_sfnt_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTTable_name.cpp"
    "${CMAKE_SOURCE_DIR}/src/sfnt/SkOTUtils.cpp"
)

# //src/sfnt:srcs
list(APPEND src_sfnt_srcs
    # Sources:
    ${src_sfnt_sfnt_srcs}
)

# //src/shaders:shader_srcs
list(APPEND src_shaders_shader_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/shaders/SkBitmapProcShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkColorFilterShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkColorShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkComposeShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkEmptyShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkImageShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkLocalMatrixShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkPerlinNoiseShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkPictureShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/SkTransformShader.cpp"
)

# //src/shaders/gradients:gradient_srcs
list(APPEND src_shaders_gradients_gradient_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/Sk4fGradientBase.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/Sk4fLinearGradient.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkGradientShader.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkGradientShaderBase.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkLinearGradient.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkRadialGradient.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkSweepGradient.cpp"
    "${CMAKE_SOURCE_DIR}/src/shaders/gradients/SkTwoPointConicalGradient.cpp"
)

# //src/shaders/gradients:srcs
list(APPEND src_shaders_gradients_srcs
    # Sources:
    ${src_shaders_gradients_gradient_srcs}
)

# //src/shaders:srcs
list(APPEND src_shaders_srcs
    # Sources:
    ${src_shaders_shader_srcs}
    ${src_shaders_gradients_srcs}
)

# //src/text:text_srcs
list(APPEND src_text_text_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/text/GlyphRun.cpp"
    "${CMAKE_SOURCE_DIR}/src/text/StrikeForGPU.cpp"
)

# //src/text:srcs
list(APPEND src_text_srcs
    # Sources:
    ${src_text_text_srcs}
)

# //src/utils:core_srcs
list(APPEND src_utils_core_srcs
    # Sources:
    "${CMAKE_SOURCE_DIR}/src/utils/SkAnimCodecPlayer.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkBase64.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCamera.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCanvasStack.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCanvasStateUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCharToGlyphCache.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkClipStackUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkCustomTypeface.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkDashPath.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkEventTracer.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkFloatToDecimal.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkGaussianColorFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkMatrix22.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkMultiPictureDocument.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkNWayCanvas.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkNullCanvas.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkOSPath.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkOrderedFontMgr.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkPaintFilterCanvas.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkParse.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkParseColor.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkParsePath.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkPatchUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkPolyUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkShadowTessellator.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkShadowUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkTextUtils.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkUTF.cpp"
    "${CMAKE_SOURCE_DIR}/src/utils/SkVMVisualizer.cpp"
)

# //src/utils:srcs
list(APPEND src_utils_srcs
    # Sources:
    ${src_utils_core_srcs}
)

# //src:srcs
list(APPEND src_srcs
    # Sources:
    ${src_android_srcs}
    ${src_codec_srcs}
    ${src_core_srcs}
    ${src_effects_srcs}
    ${src_image_srcs}
    ${src_images_srcs}
    ${src_opts_srcs}
    ${src_pathops_srcs}
    ${src_ports_srcs}
    ${src_sfnt_srcs}
    ${src_shaders_srcs}
    ${src_text_srcs}
    ${src_utils_srcs}
)

# //:skia_public
add_library(skia_public "")
target_sources(skia_public
  PRIVATE
    # Headers:
    ${include_public_hdrs}
    # Sources:
    ${include_private_hdrs}
    ${src_private_hdrs}
    ${src_srcs}
)
set_target_properties(skia_public PROPERTIES COMPILE_FLAGS
  "${DEFAULT_COMPILE_FLAGS}"
)
set_target_properties(skia_public PROPERTIES COMPILE_DEFINITIONS
  "SK_HAS_ANDROID_CODEC;SKIA_IMPLEMENTATION=1;SK_GAMMA_APPLY_TO_A8;SK_RELEASE;SK_SUPPORT_GPU=0"
)
set_target_properties(skia_public PROPERTIES INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/modules/skcms;${CMAKE_SOURCE_DIR}"
)

# //modules/skcms:skcms
add_library(modules_skcms_skcms "")
target_sources(modules_skcms_skcms
  PRIVATE
    # Headers:
    "${CMAKE_SOURCE_DIR}/modules/skcms/skcms.h"
    "${CMAKE_SOURCE_DIR}/modules/skcms/skcms.h"
    # Sources:
    "${CMAKE_SOURCE_DIR}/modules/skcms/skcms.cc"
    "${CMAKE_SOURCE_DIR}/modules/skcms/skcms_internal.h"
)
set_target_properties(modules_skcms_skcms PROPERTIES COMPILE_FLAGS
  "${DEFAULT_COMPILE_FLAGS} -O1"
)
set_target_properties(modules_skcms_skcms PROPERTIES INCLUDE_DIRECTORIES
  "${CMAKE_SOURCE_DIR}/modules/skcms;${CMAKE_SOURCE_DIR}"
)
