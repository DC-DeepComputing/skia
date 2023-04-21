"""This module contains macros to generate C++ test targets."""

load("//bazel:cc_test_with_flags.bzl", "cc_test_with_flags")
load("//bazel:cc_binary_with_flags.bzl", "cc_binary_with_flags")

CODEC_TESTS = [
    "AndroidCodecTest.cpp",
    "AnimatedImageTest.cpp",
    "CodecAnimTest.cpp",
    "CodecExactReadTest.cpp",
    "CodecPartialTest.cpp",
    "CodecRecommendedTypeTest.cpp",
    "CodecTest.cpp",
    "ColorSpaceTest.cpp",
    "EncodeTest.cpp",
    "EncodedInfoTest.cpp",
    "ExifTest.cpp",
    "GifTest.cpp",
    "IndexedPngOverflowTest.cpp",
    "WebpTest.cpp",
    "YUVTest.cpp",
]

CPU_ONLY_TESTS = [
    "AAClipTest.cpp",
    "ArenaAllocTest.cpp",
    "AsADashTest.cpp",
    "AvifTest.cpp",
    "BRDTest.cpp",
    "BadIcoTest.cpp",
    "BezierCurveTest.cpp",
    "BitSetTest.cpp",
    "BitmapCopyTest.cpp",
    "BitmapGetColorTest.cpp",
    "BitmapTest.cpp",
    "BlitMaskClip.cpp",
    "CachedDecodingPixelRefTest.cpp",
    "CanvasStateTest.cpp",
    "CanvasTest.cpp",
    "ChecksumTest.cpp",
    "ClipCubicTest.cpp",
    "ClipStackTest.cpp",
    "ClipperTest.cpp",
    "ColorMatrixTest.cpp",
    "ColorPrivTest.cpp",
    "ColorTest.cpp",
    "CtsEnforcement.cpp",
    "CubicMapTest.cpp",
    "CubicRootsTest.cpp",
    "DashPathEffectTest.cpp",
    "DataRefTest.cpp",
    "DequeTest.cpp",
    "DescriptorTest.cpp",
    "DrawBitmapRectTest.cpp",
    "DrawPathTest.cpp",
    "DrawTextTest.cpp",
    "EmptyPathTest.cpp",
    "F16StagesTest.cpp",
    "FillPathTest.cpp",
    "FitsInTest.cpp",
    "FlattenDrawableTest.cpp",
    "FlattenableFactoryToName.cpp",
    "FlattenableNameToFactory.cpp",
    "Float16Test.cpp",
    "FloatingPointTest.cpp",
    "FontHostStreamTest.cpp",
    "FontHostTest.cpp",
    "FontNamesTest.cpp",
    "FontTest.cpp",
    "FrontBufferedStreamTest.cpp",
    "GeometryTest.cpp",
    "GlyphRunTest.cpp",
    "HSVRoundTripTest.cpp",
    "HashTest.cpp",
    "HighContrastFilterTest.cpp",
    "ICCTest.cpp",
    "ImageBitmapTest.cpp",
    "ImageFrom565Bitmap.cpp",
    "ImageGeneratorTest.cpp",
    "IncrTopoSortTest.cpp",
    "InfRectTest.cpp",
    "InsetConvexPolyTest.cpp",
    "InvalidIndexedPngTest.cpp",
    "IsClosedSingleContourTest.cpp",
    "JSONTest.cpp",
    "LListTest.cpp",
    "LRUCacheTest.cpp",
    "M44Test.cpp",
    "MD5Test.cpp",
    "MallocPixelRefTest.cpp",
    "MaskCacheTest.cpp",
    "MathTest.cpp",
    "MatrixProcsTest.cpp",
    "MatrixTest.cpp",
    "MemoryTest.cpp",
    "MemsetTest.cpp",
    "MeshTest.cpp",
    "MetaDataTest.cpp",
    "MipMapTest.cpp",
    "NdkDecodeTest.cpp",
    "NdkEncodeTest.cpp",
    "NonlinearBlendingTest.cpp",
    "OSPathTest.cpp",
    "OffsetSimplePolyTest.cpp",
    "OnceTest.cpp",
    "OverAlignedTest.cpp",
    "PaintTest.cpp",
    "ParametricStageTest.cpp",
    "ParseColorTest.cpp",
    "ParsePathTest.cpp",
    "PathBuilderTest.cpp",
    "PathCoverageTest.cpp",
    "PathMeasureTest.cpp",
    "PathTest.cpp",
    "PictureBBHTest.cpp",
    "PictureShaderTest.cpp",
    "PictureTest.cpp",
    "PixelRefTest.cpp",
    "Point3Test.cpp",
    "PointTest.cpp",
    "PolyUtilsTest.cpp",
    "QuadRootsTest.cpp",
    "QuickRejectTest.cpp",
    "RRectInPathTest.cpp",
    "RTreeTest.cpp",
    "RandomTest.cpp",
    "ReadPixelsTest.cpp",
    "RecordDrawTest.cpp",
    "RecordOptsTest.cpp",
    "RecordPatternTest.cpp",
    "RecordTest.cpp",
    "RecorderTest.cpp",
    "RecordingXfermodeTest.cpp",
    "RectTest.cpp",
    "RefCntTest.cpp",
    "RegionTest.cpp",
    "RoundRectTest.cpp",
    "SRGBTest.cpp",
    "SafeMathTest.cpp",
    "SamplingTest.cpp",
    "ScalarTest.cpp",
    "ScaleToSidesTest.cpp",
    "SerialProcsTest.cpp",
    "SerializationTest.cpp",
    "ShaderOpacityTest.cpp",
    "ShaderTest.cpp",
    "ShadowTest.cpp",
    "SizeTest.cpp",
    "SkBase64Test.cpp",
    "SkBlockAllocatorTest.cpp",
    "SkColor4fTest.cpp",
    "SkColorSpaceXformStepsTest.cpp",
    "SkContainersTest.cpp",
    "SkDOMTest.cpp",
    "SkEnumBitMaskTest.cpp",
    "SkGaussFilterTest.cpp",
    "SkGlyphTest.cpp",
    "SkImageTest.cpp",
    "SkMallocTest.cpp",
    "SkPathRangeIterTest.cpp",
    "SkSLErrorTest.cpp",
    "SkSLMemoryLayoutTest.cpp",
    "SkSLTypeTest.cpp",
    "SkSharedMutexTest.cpp",
    "SkSpanTest.cpp",
    "SkStrikeCacheTest.cpp",
    "SkStrikeTest.cpp",
    "SkStringViewTest.cpp",
    "SkTBlockListTest.cpp",
    "SkUTFTest.cpp",
    "SkSLDebugTracePlayerTest.cpp",
    "SkSLDebugTraceTest.cpp",
    "SkVMTest.cpp",
    "SkVxTest.cpp",
    "Skbug6389.cpp",
    "SortTest.cpp",
    "SrcOverTest.cpp",
    "StreamTest.cpp",
    "StrikeForGPUTest.cpp",
    "StringTest.cpp",
    "StrokeTest.cpp",
    "SwizzlerTest.cpp",
    "TArrayTest.cpp",
    "TDPQueueTest.cpp",
    "TLazyTest.cpp",
    "TemplatesTest.cpp",
    "TextBlobTest.cpp",
    "TracingTest.cpp",
    "TypefaceTest.cpp",
    "UnicodeTest.cpp",
    "UtilsTest.cpp",
    "VerticesTest.cpp",
    "Writer32Test.cpp",
    "YUVCacheTest.cpp",
]

DEBUGGER_TESTS = [
    "MultiPictureDocumentTest.cpp",
    "DebugLayerManagerTest.cpp",
]

DISCARDABLE_MEMORY_POOL_TESTS = [
    "CachedDataTest.cpp",
    "DiscardableMemoryPoolTest.cpp",
    "DiscardableMemoryTest.cpp",
    "ImageCacheTest.cpp",
    "SkResourceCacheTest.cpp",
]

# These files have tests requiring a Ganesh backend, and optionally, tests only for the CPU.
# The mock Ganesh backend is always available; the other ones depend on the compile flags used
# to invoke the tests see //bazel/buildrc for these configurations.
GANESH_TESTS = [
    "AdvancedBlendTest.cpp",
    "ApplyGammaTest.cpp",
    "BackendAllocationTest.cpp",
    "BackendSurfaceMutableStateTest.cpp",
    "BlendTest.cpp",
    "BlurTest.cpp",
    "BulkRectTest.cpp",
    "ClearTest.cpp",
    "ColorFilterTest.cpp",
    "CompressedBackendAllocationTest.cpp",
    "CopySurfaceTest.cpp",
    "CullTestTest.cpp",
    "DMSAATest.cpp",
    "DefaultPathRendererTest.cpp",
    "DeviceTest.cpp",
    # "DrawOpAtlasTest.cpp",  # TODO(b/277938020): Linux - Causes SkRefCnt error after all tests finish.
    "EGLImageTest.cpp",
    "ExtendedSkColorTypeTests.cpp",
    "FindCubicConvex180ChopsTest.cpp",
    "FilterResultTest.cpp",
    "FloatingPointTextureTest.cpp",
    "GLBackendSurfaceTest.cpp",
    "GainmapShaderTest.cpp",
    "GpuDrawPathTest.cpp",
    "GpuRectanizerTest.cpp",
    "GrAHardwareBufferTest.cpp",
    # "GrClipStackTest.cpp",  # TODO(b/277938020): Linux - Fails at a Skia library assertion.
    "GrContextAbandonTest.cpp",
    "GrContextFactoryTest.cpp",
    "GrContextOOM.cpp",
    "GrDDLImageTest.cpp",
    "GrFinishedFlushTest.cpp",
    "GrGLExtensionsTest.cpp",
    # "GrGlyphVectorTest.cpp",  # TODO(b/277938020): Linux - Causes SkRefCnt error after all tests finish.
    "GrGpuBufferTest.cpp",
    "GrMemoryPoolTest.cpp",
    "GrMeshTest.cpp",
    "GrMipMappedTest.cpp",
    "GrOpListFlushTest.cpp",
    "GrPipelineDynamicStateTest.cpp",
    "GrPorterDuffTest.cpp",
    "GrQuadBufferTest.cpp",
    "GrQuadCropTest.cpp",
    "GrRenderTaskClusterTest.cpp",
    "GrSlugTest.cpp",
    "GrStyledShapeTest.cpp",
    "GrSubmittedFlushTest.cpp",
    "GrSurfaceResolveTest.cpp",
    "GrSurfaceTest.cpp",
    # "GrTextBlobTest.cpp",  # TODO(b/277938020): Linux - Causes SkRefCnt error after all tests finish.
    "GrTextureMipMapInvalidationTest.cpp",
    "GrThreadSafeCacheTest.cpp",
    "GradientTest.cpp",
    "ImageFilterCacheTest.cpp",
    # "ImageFilterTest.cpp",  # TODO(b/277938020): Linux - Fails at a Skia library assertion.
    # "ImageIsOpaqueTest.cpp",  # TODO(b/277938020): Linux - Fails with segmentation fault.
    # "ImageNewShaderTest.cpp",  # TODO(b/277938020): Linux - Fails at a test assertion.
    # "ImageTest.cpp",  # TODO(b/277938020): Linux - Fails with segmentation fault.
    "LazyProxyTest.cpp",
    "LazyStencilAttachmentTest.cpp",
    "MatrixColorFilterTest.cpp",
    "MessageBusTest.cpp",
    "OpChainTest.cpp",
    "PathRendererCacheTests.cpp",
    "PinnedImageTest.cpp",
    "PreChopPathCurvesTest.cpp",
    "PremulAlphaRoundTripTest.cpp",
    "PrimitiveProcessorTest.cpp",
    # "ProcessorTest.cpp",  # TODO(b/277938020): Linux - Fails at a test assertion.
    # "ProgramsTest.cpp",  # TODO(b/277938020): Linux - Causes SkRefCnt error after all tests finish.
    "PromiseImageTest.cpp",
    "ProxyConversionTest.cpp",
    "ProxyRefTest.cpp",
    "ProxyTest.cpp",
    "ReadWritePixelsGpuTest.cpp",
    "RectangleTextureTest.cpp",
    "RepeatedClippedBlurTest.cpp",
    "ResourceAllocatorTest.cpp",
    "ResourceCacheTest.cpp",
    # "RuntimeBlendTest.cpp",  # TODO(b/277938020): Linux - Fails at a test assertion.
    "SRGBReadWritePixelsTest.cpp",
    "SaveLayerOriginTest.cpp",
    "ShaderImageFilterTest.cpp",
    # "SkRemoteGlyphCacheTest.cpp",  # TODO(b/277938020): Linux - Fails with SkRefCnt error.
    # "SkRuntimeEffectTest.cpp",  # TODO(b/277938020): Linux - Fails at a test assertion.
    "SkSLCross.cpp",
    "SkSLDSLTest.cpp",
    "SkSLES2ConformanceTest.cpp",
    # "SkSLTest.cpp",  # TODO(b/27793802): Pixel 5 - Fails at a test assertion.
    "Skbug12214.cpp",
    # "Skbug5221.cpp",  # TODO(b/277938020): Linux - Causes SkRefCnt error after all tests finish.
    "Skbug6653.cpp",
    "SpecialImageTest.cpp",
    "SpecialSurfaceTest.cpp",
    "SrcSrcOverBatchTest.cpp",
    "SurfaceDrawContextTest.cpp",
    "SurfaceSemaphoreTest.cpp",
    # "SurfaceTest.cpp",  # TODO(b/277938020): Linux - Fails at a test assertion.
    "TestTest.cpp",
    "TextureBindingsResetTest.cpp",
    "TextureOpTest.cpp",
    "TextureProxyTest.cpp",
    "TextureStripAtlasManagerTest.cpp",
    "TopoSortTest.cpp",
    "TraceMemoryDumpTest.cpp",
    "TransferPixelsTest.cpp",
    "TriangulatingPathRendererTests.cpp",
    "VkBackendSurfaceTest.cpp",
    "VkHardwareBufferTest.cpp",
    "VkPriorityExtensionTest.cpp",
    "VkProtectedContextTest.cpp",
    "VkWrapTests.cpp",
    "VkYcbcrSamplerTest.cpp",
    "WangsFormulaTest.cpp",
    "WindowRectanglesTest.cpp",
    "WrappedSurfaceCopyOnWriteTest.cpp",
    "WritePixelsTest.cpp",
]

PATHOPS_TESTS = [
    "CubicChopTest.cpp",
    "PathOpsAngleIdeas.cpp",
    "PathOpsAngleTest.cpp",
    "PathOpsAsWindingTest.cpp",
    "PathOpsBattles.cpp",
    "PathOpsBoundsTest.cpp",
    "PathOpsBuilderConicTest.cpp",
    "PathOpsBuilderTest.cpp",
    "PathOpsBuildUseTest.cpp",
    "PathOpsChalkboardTest.cpp",
    "PathOpsConicIntersectionTest.cpp",
    "PathOpsConicLineIntersectionTest.cpp",
    "PathOpsConicQuadIntersectionTest.cpp",
    "PathOpsCubicConicIntersectionTest.cpp",
    "PathOpsCubicIntersectionTest.cpp",
    "PathOpsCubicLineIntersectionIdeas.cpp",
    "PathOpsCubicLineIntersectionTest.cpp",
    "PathOpsCubicQuadIntersectionTest.cpp",
    "PathOpsCubicReduceOrderTest.cpp",
    "PathOpsDCubicTest.cpp",
    "PathOpsDLineTest.cpp",
    "PathOpsDPointTest.cpp",
    "PathOpsDRectTest.cpp",
    "PathOpsDVectorTest.cpp",
    "PathOpsExtendedTest.cpp",
    "PathOpsFuzz763Test.cpp",
    "PathOpsInverseTest.cpp",
    "PathOpsIssue3651.cpp",
    "PathOpsLineIntersectionTest.cpp",
    "PathOpsLineParametetersTest.cpp",
    "PathOpsOpCircleThreadedTest.cpp",
    "PathOpsOpCubicThreadedTest.cpp",
    "PathOpsOpLoopThreadedTest.cpp",
    "PathOpsOpRectThreadedTest.cpp",
    "PathOpsOpTest.cpp",
    "PathOpsQuadIntersectionTest.cpp",
    "PathOpsQuadLineIntersectionTest.cpp",
    "PathOpsQuadLineIntersectionThreadedTest.cpp",
    "PathOpsQuadReduceOrderTest.cpp",
    "PathOpsSimplifyDegenerateThreadedTest.cpp",
    "PathOpsSimplifyFailTest.cpp",
    "PathOpsSimplifyQuadralateralsThreadedTest.cpp",
    "PathOpsSimplifyQuadThreadedTest.cpp",
    "PathOpsSimplifyRectThreadedTest.cpp",
    "PathOpsSimplifyTest.cpp",
    "PathOpsSimplifyTrianglesThreadedTest.cpp",
    "PathOpsSkpTest.cpp",
    "PathOpsThreeWayTest.cpp",
    "PathOpsTigerTest.cpp",
    "PathOpsTightBoundsTest.cpp",
    "PathOpsTypesTest.cpp",
    "StrokerTest.cpp",
]

PDF_TESTS = [
    "AnnotationTest.cpp",
    "PDFDeflateWStreamTest.cpp",
    "PDFDocumentTest.cpp",
    "PDFGlyphsToUnicodeTest.cpp",
    "PDFJpegEmbedTest.cpp",
    "PDFMetadataAttributeTest.cpp",
    "PDFOpaqueSrcModeToSrcOverTest.cpp",
    "PDFPrimitivesTest.cpp",
    "PDFTaggedLinkTest.cpp",
    "PDFTaggedPruningTest.cpp",
    "PDFTaggedTableTest.cpp",
    "PDFTaggedTest.cpp",
    # TODO(kjlubick, bungeman) There are PDF-specific tests in
    # CanvasTest.cpp
]

def skia_cpu_tests(
        name,
        tests,
        harness,
        resources = [],
        flags = {},
        extra_deps = [],
        limit_to = []):
    """Defines tests that should run only with --config=cpu

    This macro will create one cc_test_with_flags rule for each file in tests.
    These tests are configured to use the BazelTestRunner and actually run all tests
    (e.g. those defined with DEF_TEST) in the file if no GPU backend has been specified,
    that is, the bazel test command was invoked with --config=cpu (see buildrc for this
    definition).

    This macro cannot un-define those targets completely if a GPU backend has been specified
    because the macro runs before the config flags are available for use. An analogy is a
    C++ macro cannot do a #if on the value of a C++ variable.

    Instead, if a GPU backend has been specified (e.g. --config=gl), we only compile the
    BazelNoopRunner.cpp and nothing else (e.g. no deps, not even Skia). This makes any CPU tests
    a trivially passing executable, which can easily be cached and effectively skipped.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_TEST (see Test.h).
        harness: A label (string) corresponding to a skia_cc_library which all the supplementary
                 test helpers, utils, etc. (e.g. Test.h and Test.cpp). Ideally, this is as small
                 as feasible, to avoid unnecessary bloat or recompilation if something unrelated
                 changes.
        resources: A label corresponding to a file_group target that has any skia resource files
                   (e.g. images, fonts) needed to run these tests. Resources change infrequently,
                   so it's not super important that this be a precise list.
        flags: A map of strings to lists of strings to specify features that must be compiled in
               for these tests to work. For example, tests targeting our codec logic will want the
               various codecs included, but most tests won't need that.
        extra_deps: A list of labels corresponding to skia_cc_library targets which this test needs
                    to work, typically some utility located under //tools
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
                  restrict where this test will be compiled and ran. If the list is empty, it will
                  run anywhere. If it is non-empty, it will only run on platforms which match the
                  entire set of constraints. See https://github.com/bazelbuild/platforms for these.
    """
    test_targets = []
    for filename in tests:
        new_target = filename[:-4]  # trim .cpp
        test_targets.append(new_target)
        cc_test_with_flags(
            name = new_target,
            size = "small",
            srcs = select({
                # Make this a no-op test if compiling with a GPU backend.
                ":skip_cpu_tests": ["BazelNoopRunner.cpp"],
                "//conditions:default": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
            }),
            deps = select({
                ":skip_cpu_tests": [],
                # Only build and apply deps if we have a no-op test.
                "//conditions:default": [
                    harness,
                    "//:skia_internal",
                ] + extra_deps,
            }),
            data = resources,
            set_flags = flags,
            target_compatible_with = limit_to,
        )

    # https://bazel.build/reference/be/general#test_suite
    native.test_suite(
        name = name,
        tests = test_targets,
    )

def skia_ganesh_tests(
        name,
        tests,
        harness,
        resources = [],
        flags = {},
        extra_deps = [],
        limit_to = []):
    """Defines tests that should run only when a Ganesh GPU backend is compiled in, e.g --config=gl

    This macro will create one cc_test_with_flags rule for each file in tests.
    These tests are configured to use the BazelTestRunner and actually run all tests (e.g. those
    defined with DEF_TEST or DEF_GANESH_TEST) in the file if a Ganesh GPU backend has been
    specified, that is, the bazel test command was invoked with --config=gl or --config=vk
    (see buildrc for this definition).

    This macro cannot un-define those targets completely if only the CPU backend is being used
    because the macro runs before the config flags are available for use. An analogy is a
    C++ macro cannot do a #if on the value of a C++ variable.

    Instead, if a Ganesh GPU backend has not been specified (e.g. --config=cpu), we only compile the
    BazelNoopRunner.cpp and nothing else (e.g. no deps, not even Skia). This makes any Ganesh GPU
    tests a trivially passing executable, which can easily be cached and effectively skipped.

    Test files may have both CPU tests (i.e. DEF_TEST) and GPU tests (e.g. DEF_GANESH_TEST). The
    BazelTestRunner will always run all tests that were compiled in. The CPU tests defined in this
    way will *not* be run with --config=cpu because they live in a file that can only be compiled
    with at least one GPU backend; they will be run with a config like --config=gl, along side the
    GPU tests defined with DEF_GANESH_TEST.

    Args:
        name: The name of the test_suite that groups these tests together.
        tests: A list of strings, corresponding to C++ files with one or more DEF_GANESH_TEST
               (see Test.h).
        harness: A label (string) corresponding to a skia_cc_library which all the supplementary
                 test helpers, utils, etc. (e.g. Test.h and Test.cpp). Ideally, this is as small
                 as feasible, to avoid unnecessary bloat or recompilation if something unrelated
                 changes.
        resources: A label corresponding to a file_group target that has any skia resource files
                   (e.g. images, fonts) needed to run these tests. Resources change infrequently,
                   so it's not super important that this be a precise list.
        flags: A map of strings to lists of strings to specify features that must be compiled in
               for these tests to work. For example, tests targeting our codec logic will want the
               various codecs included, but most tests won't need that.
        extra_deps: A list of labels corresponding to skia_cc_library targets which this test needs
                    to work, typically some utility located under //tools
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
                  restrict where this test will be compiled and ran. If the list is empty, it will
                  run anywhere. If it is non-empty, it will only run on platforms which match the
                  entire set of constraints. See https://github.com/bazelbuild/platforms for these.
    """
    test_targets = []
    for filename in tests:
        new_target = filename[:-4]  # trim .cpp
        test_targets.append(new_target)
        cc_test_with_flags(
            name = new_target,
            size = "small",
            srcs = select({
                "//src/gpu:has_gpu_backend": [
                    "BazelTestRunner.cpp",
                    filename,
                ],
                # Make this a no-op test if not compiling with a Ganesh GPU backend.
                "//conditions:default": ["BazelNoopRunner.cpp"],
            }),
            deps = select({
                # Only build and apply deps if we have a no-op test.
                "//src/gpu:has_gpu_backend": [
                    harness,
                    "//:skia_internal",
                ] + extra_deps,
                "//conditions:default": [],
            }),
            data = resources,
            set_flags = flags,
            target_compatible_with = limit_to,
            tags = [
                # We currently have no RBE machines with GPUs, so we cannot run these remotely.
                "no-remote",
            ],
        )

    # https://bazel.build/reference/be/general#test_suite
    native.test_suite(
        name = name,
        tests = test_targets,
    )

def skia_test(
        name,
        srcs,
        deps,
        requires_resources_dir = False,
        extra_args = [],
        flags = {},
        limit_to = [],
        tags = [],
        size = None):
    """Defines a generic Skia C++ unit test.

    This macro produces a <name>_binary C++ binary and a <name>.sh wrapper script that runs the
    binary with the desired command-line arguments (see the extra_args and requires_resources_dir
    arguments). The <name>.sh wrapper script is exposed as a Bazel test target via the sh_target
    rule.

    The reason why we place command-line arguments in a wrapper script is that it makes it easier
    to run a Bazel-built skia_test outside of Bazel. This is useful e.g. for CI jobs where we want
    to perform test compilation and execution as different steps on different hardware (e.g.
    compile on a GCE machine, run tests on a Skolo device). In this scenario, the test could be
    executed outside of Bazel by simply running the <name>.sh script without any arguments. See the
    skia_android_test macro for an example.

    Note: The srcs attribute must explicitly include a test runner (e.g.
    //tests/BazelTestRunner.cpp).

    Args:
        name: The name of the test.
        srcs: C++ source files.
        deps: C++ library dependencies.
        requires_resources_dir: Indicates whether this test requires any files under //resources,
            such as images, fonts, etc. If so, the compiled C++ binary will be invoked with flag
            --resourcePath set to the path to the //resources directory under the runfiles tree.
            Note that this implies the test runner must recognize the --resourcePath flag for this
            to work.
        extra_args: Any additional command-line arguments to pass to the compiled C++ binary.
        flags: A map of strings to lists of strings to specify features that must be compiled in
            for these tests to work. For example, tests targeting our codec logic will want the
            various codecs included, but most tests won't need that.
        limit_to: A list of platform labels (e.g. @platform//os:foo; @platform//cpu:bar) which
            restrict where this test will be compiled and ran. If the list is empty, it will run
            anywhere. If it is non-empty, it will only run on platforms which match the entire set
            of constraints. See https://github.com/bazelbuild/platforms for these.
        tags: A list of tags for the generated test target.
        size: The size of the test.
    """
    test_binary = "%s_binary" % name

    # We compile the test as a cc_binary, rather than as as a cc_test, because we will not
    # "bazel test" this binary directly. Instead, we will "bazel test" a wrapper script that
    # invokes this binary with the required command-line parameters.
    cc_binary_with_flags(
        name = test_binary,
        srcs = srcs,
        deps = deps,
        data = ["//resources"] if requires_resources_dir else [],
        set_flags = flags,
        target_compatible_with = limit_to,
        testonly = True,  # Needed to gain access to test-only files.
    )

    test_runner = "%s.sh" % name

    test_args = ([
        "--resourcePath",
        "$$(realpath $$(dirname $(rootpath //resources:README)))",
    ] if requires_resources_dir else []) + extra_args

    native.genrule(
        name = "%s_runner" % name,
        srcs = [test_binary] + (
            # The script template computes the path to //resources under the runfiles tree via
            # $$(dirname $(rootpath //resources:README)), so we need to list //resources:README
            # here explicitly. This file was chosen arbitrarily; there is nothing special about it.
            ["//resources", "//resources:README"] if requires_resources_dir else []
        ),
        outs = [test_runner],
        cmd = "echo '%s' > $@" % _TEST_RUNNER_TEMPLATE.format(
            test_binary = test_binary,
            test_args = "\\\n    ".join(test_args),
        ),
        testonly = True,
    )

    native.sh_test(
        name = name,
        size = size,
        srcs = [test_runner],
        data = [test_binary] + (["//resources"] if requires_resources_dir else []),
        tags = tags,
    )

# This test runner might run on Android devices, which typically do not have a /bin/bash binary.
_TEST_RUNNER_TEMPLATE = """#!/bin/sh
$(rootpath {test_binary}) {test_args}
"""

# Device names recognized by the skia_android_test macro.
# TODO(lovisolo): Add more devices.
PIXEL_5 = "pixel_5"
PIXEL_7 = "pixel_7"

# Named test groups recognized by the skia_android_test macro.
# TODO(lovisolo): Add more test groups.
CODEC = "codecs"
GANESH = "ganesh"
PATHOPS = "pathops"

def skia_android_test(name, device, test_group, flavor = None):
    """Defines a Skia Android test.

    Note: This macro is designed to facilitate generating test targets from
    //infra/bots/gen_tasks_logic/gen_tasks_logic.go, but can also be used for hand-written targets.

    This macro takes a device name (e.g. "pixel_5"), a named group of C++ unit tests (e.g.
    "ganesh") and a flavor (e.g. "vulkan"), and produces a skia_test target with label
    <name>_real_test. This target compiles the C++ unit tests, along with
    //tests/BazelTestRunner.cpp, with a set of flags appropriate for the given combination of
    device, tests and flavor. These flags include //command_line_option:platforms, which is
    set to @platforms//os:android. This allows us to "bazel build" the target without passing any
    additional command-line arugments to Bazel.

    This macro also generates a <name> sh_test that runs the skia_test on an attached Android
    device via ADB. This target works as follows:

     - It produces a <name>.tar.gz tarball containing the C++ binary and wrapper script produced by
       the skia_test rule, and optionally the contents of the //resources directory.
     - It produces a <name>.sh script that copies the tarball into the attached Android device via
       ADB, extracts it, runs the test on the device, cleans up the copied files, and pipes
       through the test's exit code.
     - When executed via "bazel test", it runs the <name>.sh script and reports the test results.

    The <name> target can be executed with "bazel test" on a developer workstation if one of these
    conditions is met:

     - The machine has an Android device attached via USB and "adb" is installed.
     - TCP port 5037 is forwarded from a different machine running an ADB server with an attached
       Android device, such as a Raspberry Pi leased from the Skolo.

    For CI jobs, it is possible to perform test compilation and test execution as separate steps.
    For example, test compilation can happen on a GCE machine via "bazel build <name>", and test
    execution can happen on a Raspberry Pi attached to an appropriate Android device. The latter
    requires the <name>.tar.gz and <name>.sh artifacts generated from the previous step, and test
    execution would be as simple as running the <name>.sh script with no arguments.

    Note: Although not currently supported, we could use a similar approach for Apple devices in
    the future.

    Args:
        name: The name of the test.
        device: The device that this target should run on (e.g. "pixel_5").
        test_group: Which named collection of C++ tests to run (e.g. "ganesh").
        flavor: A named variant of the test (e.g. "vulkan"). Optional.
    """

    # These delegate functions determine what gets tested and how.
    srcs = _skia_android_test_srcs(device, test_group, flavor)
    deps = _skia_android_test_deps(device, test_group, flavor)
    flags = _skia_android_test_flags(device, test_group, flavor)
    extra_args = _skia_android_test_extra_args(device, test_group, flavor)
    requires_resources_dir = _skia_android_test_requires_resources_dir(device, test_group, flavor)

    skia_test(
        name = "%s_real_test" % name,
        srcs = srcs,
        deps = deps,
        flags = flags,
        extra_args = extra_args,
        requires_resources_dir = requires_resources_dir,
        tags = [
            # Exclude it from wildcards, e.g. "bazel test //...". We never want to run this binary
            # directly.
            "manual",
            "no-remote",  # RBE workers cannot run Android tests.
        ],
        size = "large",  # Can take several minutes.
    )

    test_binary = "%s_real_test_binary" % name
    test_runner = "%s_real_test.sh" % name

    archive = "%s_archive" % name
    archive_srcs = [test_runner, test_binary] + (
        ["//resources"] if requires_resources_dir else []
    )

    # Create an archive containing the test and its resources, with a structure that emulates
    # the environment expected by the test when executed via "bazel test". This archive can be
    # pushed to an Android device via "adb push", and the contained test can be executed on the
    # device via "adb shell" as long as the working directory is set to the directory where the
    # archive is extracted.
    #
    # See https://bazel.build/reference/test-encyclopedia#initial-conditions.
    native.genrule(
        name = archive,
        srcs = archive_srcs,
        outs = ["%s.tar.gz" % name],
        cmd = "\\\n".join([
            # Create archive root directory, and ensure it is cleaned up on exit, even in the
            # case of errors.
            "ARCHIVE_DIR=$$(mktemp -d);",
            "trap \"rm -rf $$ARCHIVE_DIR\" EXIT;",

            # execpaths point to physical files generated by Bazel (e.g.
            # bazel-out/k8-linux_x64-dbg/bin/tests/some_test), whereas rootpaths are the paths
            # that a binary running via "bazel run" or "bazel test" expects (e.g
            # tests/some_test). Thus, we must map the former to the latter.
            #
            # Reference:
            # https://bazel.build/reference/be/make-variables#predefined_label_variables
            "EXECPATHS=(%s);" % " ".join(["$(execpaths %s)" % src for src in archive_srcs]),
            "ROOTPATHS=(%s);" % " ".join(["$(rootpaths %s)" % src for src in archive_srcs]),
            "if [ $${#EXECPATHS[*]} -ne $${#ROOTPATHS[*]} ]; then",
            "    echo EXECPATHS and ROOTPATHS have different lengths;",
            "    exit 1;",
            "fi;",

            # Copy each file in EXECPATHS into ARCHIVE_DIR, renamed as its corresponding entry
            # in ROOTPATHS.
            "for (( i=0; i<$${#EXECPATHS[*]}; i++ )); do",
            "    EXECPATH=$${EXECPATHS[$$i]};",
            "    ROOTPATH=$${ROOTPATHS[$$i]};",
            "    mkdir -p $$ARCHIVE_DIR/$$(dirname $$ROOTPATH);",
            "    cp $$EXECPATH $$ARCHIVE_DIR/$$ROOTPATH;",
            "done;",

            # Create archive.
            "tar zcf $@ -C $$ARCHIVE_DIR .;",
        ]),
        testonly = True,  # Needed to gain access to test-only files.
    )

    adb_test_runner = "%s.sh" % name

    native.genrule(
        name = "%s_runner" % name,
        srcs = [archive, test_binary],
        outs = [adb_test_runner],
        cmd = "echo '%s' > $@" % _ADB_TEST_RUNNER_TEMPLATE.format(
            archive = archive,
            test_binary = test_binary,
        ),
        testonly = True,
    )

    native.sh_test(
        name = name,
        size = "small",
        srcs = [adb_test_runner],
        data = [archive],
        tags = [
            # Exclude it from wildcards, e.g. "bazel test //...". We cannot assume that the machine
            # Bazel is running on is attached to an appropriate Android device.
            #
            # TODO(lovisolo): Use platforms and target_compatible_with to achieve this.
            "manual",
            # This test runner assumes there is an appropriate Android device connected via
            # USB, therefore it cannot run on RBE.
            "no-remote",
        ],
    )

def _skia_android_test_srcs(device, test_group, flavor):  # buildifier: disable=unused-variable
    srcs = ["BazelTestRunner.cpp"]
    if test_group == CODEC:
        srcs += CODEC_TESTS
    elif test_group == GANESH:
        srcs += GANESH_TESTS
    elif test_group == PATHOPS:
        srcs += PATHOPS_TESTS
    else:
        fail("Unknown test_group:", test_group)
    return srcs

def _skia_android_test_deps(device, test_group, flavor):  # buildifier: disable=unused-variable
    deps = ["//:skia_internal"]
    if test_group == CODEC:
        deps.append(":tests_base")
    elif test_group == GANESH:
        deps.append(":tests_base")
    elif test_group == PATHOPS:
        deps.append(":pathops_tests_base")
    else:
        fail("Unknown test_group:", test_group)
    return deps

def _skia_android_test_flags(device, test_group, flavor):  # buildifier: disable=unused-variable
    flags = {
        "platforms": [
            # This can be decided based on the device.
            "//bazel/platform:android_arm64",
        ],
    }
    if test_group == CODEC:
        flags.update({
            "include_decoder": [
                "avif_decode_codec",
                "gif_decode_codec",
                "jpeg_decode_codec",
                "jxl_decode_codec",
                "png_decode_codec",
                "raw_decode_codec",
                "webp_decode_codec",
            ],
            "include_encoder": [
                "jpeg_encode_codec",
                "png_encode_codec",
                "webp_encode_codec",
            ],
            "gpu_backend": ["gl_backend"],
            "enable_gpu_test_utils": ["true"],
        })
    elif test_group == GANESH:
        flags.update({
            "gpu_backend": ["gl_backend"],
            "enable_gpu_test_utils": ["true"],
        })
    elif test_group == PATHOPS:
        flags.update({
            "gpu_backend": ["gl_backend"],
            "enable_gpu_test_utils": ["true"],
        })
    else:
        fail("Unknown test_group:", test_group)
    return flags

def _skia_android_test_extra_args(device, test_group, flavor):  # buildifier: disable=unused-variable
    # This is a good place to compute a list of tests to skip (e.g. via a --skip) flag.
    return []

def _skia_android_test_requires_resources_dir(device, test_group, flavor):  # buildifier: disable=unused-variable
    requires_resources_dir = False
    if test_group == CODEC:
        requires_resources_dir = True
    elif test_group == GANESH:
        requires_resources_dir = True
    elif test_group == PATHOPS:
        requires_resources_dir = False
    else:
        fail("Unknown test_group:", test_group)
    return requires_resources_dir

# Curly braces are escaped ("{" becomes "{{", "}" becomes "}}") because we will format this
# template with https://bazel.build/rules/lib/string#format.
#
# This script can be customized based on the device.
_ADB_TEST_RUNNER_TEMPLATE = """#!/bin/bash

# TODO(lovisolo): Should we check that the machine is attached to the expected device type? E.g.
#                 run "adb devices -l" and check that the output contains "model:Pixel_5".

# Note: this script was only tested on a Pixel 5. The following comments refer to findings on said
# device, and might not be true on other devices.
#
# The /sdcard/revenge_of_the_skiabot directory is writable for non-root users, but files in this
# directory cannot be executed. For this reason, we extract the archive in a directory under /data,
# which allows executing files but requires root privileges.
#
# TODO(lovisolo): Can we do this without "su"-ing as root? Does this work on non-rooted devices?
# TODO(lovisolo): Test on more devices.
ARCHIVE_ON_DEVICE=/sdcard/revenge_of_the_skiabot/bazel-adb-test.tar.gz
DIRECTORY_ON_DEVICE=/data/bazel-adb-test

# Print commands and expand variables for easier debugging.
set -x

# Ensure that we clean up the device on exit, even in the case of failures.
trap "adb shell su root rm -rf $${{ARCHIVE_ON_DEVICE}} $${{DIRECTORY_ON_DEVICE}}" EXIT

# Upload archive.
adb push $(rootpath {archive}) $${{ARCHIVE_ON_DEVICE}}

# Extract archive.
adb shell su root mkdir $${{DIRECTORY_ON_DEVICE}}
adb shell su root tar xzvf $${{ARCHIVE_ON_DEVICE}} -C $${{DIRECTORY_ON_DEVICE}}

# Run test inside the directory where the archive was extracted. We set the working directory to
# the root of the archive, which emulates the directory structure expected by the test when invoked
# with "bazel test". See https://bazel.build/reference/test-encyclopedia#initial-conditions.
echo "cd $${{DIRECTORY_ON_DEVICE}} && $(rootpath {test_binary})" | adb shell su root
"""
