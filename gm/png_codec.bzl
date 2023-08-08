"""This module defines the png_codec_tests macro."""

load("//bazel:cc_test_with_flags.bzl", "cc_test_with_flags")

# These lists of lists are shaped as follows:
#
#     [images, surface_config, decode_mode, dst_color_type, dst_alpha_type]
#
# For each such list, we will define a test that decodes each image into an SkImage using
# SkPngDecoder. The decode mode, destination color type and alpha type are specified via the
# decode_mode, dst_color_type and dst_alpha_type fields, respectively. The resulting image is then
# drawn into an SkSurface specified via the surface_config field, and is saved as an undeclared
# test output which may be uploaded to Gold. See //gm/BazelGMRunner.cpp for more details.
_GRAYSCALE_TESTS = [
    [
        "@skimage//:dm_pngs_gray8_opaque",
        surface_config,
        decode_mode,
        dst_color_type,
        "premul",
    ]
    for surface_config in ["8888", "565"]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
    for dst_color_type in ["get-from-canvas"] + (
        # Skip meaningless combinations.
        [
            "force-grayscale",
            "force-nonnative-premul-color",
        ] if surface_config != "565" else []
    )
]
_COLOR_TRANSLUCENT_TESTS = [
    [
        "@skimage//:dm_pngs_rgba8888_translucent",
        "8888",  # We exclude 565 because it is meaningless for all below dst_color_types.
        decode_mode,
        dst_color_type,
        dst_alpha_type,
    ]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
    for dst_color_type in ["get-from-canvas", "force-nonnative-premul-color"]
    for dst_alpha_type in ["unpremul", "premul"]
]
_COLOR_OPAQUE_TESTS = [
    [
        "@skimage//:dm_pngs_rgba8888_opaque",
        surface_config,
        decode_mode,
        dst_color_type,
        "premul",
    ]
    for surface_config in ["8888", "565"]
    for decode_mode in ["get-all-pixels", "incremental", "zero-init"]
    for dst_color_type in ["get-from-canvas"] + (
        # Skip meaningless combinations.
        ["force-nonnative-premul-color"] if surface_config != "565" else []
    )
]
_TESTS = _GRAYSCALE_TESTS + _COLOR_TRANSLUCENT_TESTS + _COLOR_OPAQUE_TESTS

def png_codec_tests(name):
    """Generates various cc_test_with_flags targets for png_codec.cpp.

    Args:
        name: The name of the test_suite to generate.
    """

    all_tests = []

    for images, surface_config, decode_mode, dst_color_type, dst_alpha_type in _TESTS:
        test_name = "png_codec_%s_%s_%s_%s_%s_test" % (
            images.replace("@skimage//:dm_pngs_", "").replace("_", "-"),
            surface_config,
            decode_mode,
            dst_color_type,
            dst_alpha_type,
        )
        all_tests.append(test_name)

        cc_test_with_flags(
            name = test_name,
            size = "large",
            srcs = [
                "BazelGMRunner.cpp",
                "png_codec.cpp",
            ],
            args = [
                "--surfaceConfig",
                surface_config,
                "--pngCodecGMImages",
                "external/skimage/dm",
                "--pngCodecDecodeMode",
                decode_mode,
                "--pngCodecDstColorType",
                dst_color_type,
                "--pngCodecDstAlphaType",
                dst_alpha_type,
            ],
            data = [images],
            set_flags = {
                "include_decoder": [
                    "png_decode_codec",
                ],
            },
            deps = [":tests_base"],
        )

    native.test_suite(
        name = name,
        tests = all_tests,
    )

    # List all generated target names for greppability.
    #
    # Editing this list does not by itself affect which targets are generated. Instead, edit the
    # list comprehensions at the top of this file, try to run a target with Bazel, and update this
    # list as instructed in the "out of sync" error message produced by the below fail() statement.
    greppable_test_list = [
        "png_codec_gray8-opaque_8888_get-all-pixels_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_8888_get-all-pixels_force-grayscale_premul_test",
        "png_codec_gray8-opaque_8888_get-all-pixels_force-nonnative-premul-color_premul_test",
        "png_codec_gray8-opaque_8888_incremental_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_8888_incremental_force-grayscale_premul_test",
        "png_codec_gray8-opaque_8888_incremental_force-nonnative-premul-color_premul_test",
        "png_codec_gray8-opaque_8888_zero-init_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_8888_zero-init_force-grayscale_premul_test",
        "png_codec_gray8-opaque_8888_zero-init_force-nonnative-premul-color_premul_test",
        "png_codec_gray8-opaque_565_get-all-pixels_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_565_incremental_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_565_zero-init_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_get-all-pixels_get-from-canvas_unpremul_test",
        "png_codec_rgba8888-translucent_8888_get-all-pixels_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_get-all-pixels_force-nonnative-premul-color_unpremul_test",
        "png_codec_rgba8888-translucent_8888_get-all-pixels_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-translucent_8888_incremental_get-from-canvas_unpremul_test",
        "png_codec_rgba8888-translucent_8888_incremental_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_incremental_force-nonnative-premul-color_unpremul_test",
        "png_codec_rgba8888-translucent_8888_incremental_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-translucent_8888_zero-init_get-from-canvas_unpremul_test",
        "png_codec_rgba8888-translucent_8888_zero-init_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_zero-init_force-nonnative-premul-color_unpremul_test",
        "png_codec_rgba8888-translucent_8888_zero-init_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_8888_get-all-pixels_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_8888_get-all-pixels_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_8888_incremental_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_8888_incremental_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_8888_zero-init_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_8888_zero-init_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_565_get-all-pixels_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_565_incremental_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_565_zero-init_get-from-canvas_premul_test",
    ]
    if greppable_test_list != all_tests:
        msg = [
            "Variable greppable_test_list is out of sync. Please update it as follows:",
            "",
            "    greppable_test_list = [",
        ] + [
            "        \"" + test + "\","
            for test in all_tests
        ] + [
            "    ]",
        ]
        fail("\n".join(msg))
