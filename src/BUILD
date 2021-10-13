package(default_visibility = ["//:__subpackages__"])

filegroup(
    name = "codec-hdrs",
    srcs = glob([
        "codec/*.h",
    ]),
)

filegroup(
    name = "codec-srcs",
    srcs = glob([
        "codec/SkBmp*.cpp",
        "codec/SkCodec.cpp",
        "codec/SkCodecImageGenerator.cpp",
        "codec/SkColorTable.cpp",
        "codec/SkMasks.cpp",
        "codec/SkMaskSwizzler.cpp",
        "codec/SkSampler.cpp",
        "codec/SkSwizzler.cpp",
        "codec/SkWbmpCodec.cpp",
    ]),
)

filegroup(
    name = "core-hdrs",
    srcs = glob([
        "core/*.h",
    ]),
)

filegroup(
    name = "core-srcs",
    srcs = glob([
        "core/*.cpp",
    ]),
)

filegroup(
    name = "gpu-hdrs",
    srcs = glob([
        "gpu/**/*.h",
    ]),
)

filegroup(
    name = "gpu-srcs",
    srcs = glob([
        "gpu/*.cpp",
        "gpu/effects/*.cpp",
        "gpu/geometry/*.cpp",
        "gpu/glsl/*.cpp",
        "gpu/gradients/*.cpp",
        "gpu/mock/*.cpp",
        "gpu/ops/*.cpp",
        "gpu/tessellate/**/*.cpp",
        "gpu/text/*.cpp",
    ]),
)

filegroup(
    name = "gpu-webgl",
    srcs = glob(
        [
            "gpu/gl/**/*.h",
            "gpu/gl/*.cpp",
            "gpu/gl/builders/*.cpp",
            "gpu/gl/webgl/*.cpp",
            "gpu/v1/*.cpp",
            "gpu/v1/*.h",
        ],
        exclude = [
            "gpu/gl/GrGLMakeNativeInterface_none.cpp",
        ],
    ),
)

filegroup(
    name = "effects-hdrs",
    srcs = glob([
        "effects/**/*.h",
    ]),
)

filegroup(
    name = "effects-srcs",
    srcs = glob([
        "effects/**/*.cpp",
    ]),
)

filegroup(
    name = "image-hdrs",
    srcs = glob([
        "image/*.h",
    ]),
)

filegroup(
    name = "image-srcs",
    srcs = glob([
        "image/*.cpp",
    ]),
)

filegroup(
    name = "images-hdrs",
    srcs = glob([
        "images/*.h",
    ]),
)

filegroup(
    name = "images-srcs",
    srcs = glob([
        "images/*.cpp",
    ]),
)

filegroup(
    name = "opts-hdrs",
    srcs = glob([
        "opts/*.h",
    ]),
)

filegroup(
    name = "pathops-hdrs",
    srcs = glob([
        "pathops/*.h",
    ]),
)

filegroup(
    name = "pathops-srcs",
    srcs = glob([
        "pathops/*.cpp",
    ]),
)

filegroup(
    name = "sfnt-hdrs",
    srcs = glob([
        "sfnt/*.h",
    ]),
)

filegroup(
    name = "sfnt-srcs",
    srcs = glob([
        "sfnt/*.cpp",
    ]),
)

filegroup(
    name = "shaders-hdrs",
    srcs = glob([
        "shaders/**/*.h",
    ]),
)

filegroup(
    name = "shaders-srcs",
    srcs = glob([
        "shaders/**/*.cpp",
    ]),
)

filegroup(
    name = "sksl-hdrs",
    srcs = glob([
        "sksl/**/*.h",
    ]),
)

filegroup(
    name = "sksl-srcs",
    srcs = glob(
        include = [
            "sksl/**/*.cpp",
        ],
        exclude = [
            "sksl/lex/Main.cpp",
            "sksl/SkSLMain.cpp",
        ],
    ),
)

filegroup(
    name = "sksl-txts",
    srcs = [
        "sksl/generated/sksl_frag.dehydrated.sksl",
        "sksl/generated/sksl_gpu.dehydrated.sksl",
        "sksl/generated/sksl_public.dehydrated.sksl",
        "sksl/generated/sksl_rt_shader.dehydrated.sksl",
        "sksl/generated/sksl_vert.dehydrated.sksl",
    ],
)

filegroup(
    name = "utils-hdrs",
    srcs = glob([
        "utils/*.h",
    ]),
)

filegroup(
    name = "utils-srcs",
    srcs = glob(
        include = [
            "utils/*.cpp",
        ],
        exclude = ["utils/SkShaperJSONWriter.cpp"],
    ),
)
