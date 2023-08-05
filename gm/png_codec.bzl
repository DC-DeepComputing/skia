"""This module defines the png_codec_tests macro."""

load("//bazel:cc_test_with_flags.bzl", "cc_test_with_flags")

# List of lists of shape [images, surface_config, mode, dst_color_type, dst_alpha_type].
#
# TODO(lovisolo): Add more surface_configs.
_TESTS = [
    # Grayscale images.
    ["@skimage//:dm_pngs_gray8_opaque", surface_config, mode, dst_color_type, "premul"]
    for surface_config in ["8888", "565"]
    for mode in ["codec", "codec-zero-init", "scanline"]
    for dst_color_type in ["get-from-canvas"] + (
        # Skip uninteresting combinations.
        [
            "force-grayscale",
            "force-nonnative-premul-color",
        ] if surface_config != "565" else []
    )
] + [
    # Color translucent images.
    ["@skimage//:dm_pngs_rgba8888_translucent", surface_config, mode, dst_color_type, dst_alpha_type]
    for surface_config in [
        # We exclude 565 because it yields uninteresting test cases for all dst_color_types.
        "8888",
    ]
    for mode in ["codec", "codec-zero-init", "scanline"]
    for dst_color_type in ["get-from-canvas", "force-nonnative-premul-color"]
    for dst_alpha_type in ["unpremul", "premul"]
] + [
    # Color opaque images.
    ["@skimage//:dm_pngs_rgba8888_opaque", surface_config, mode, dst_color_type, "premul"]
    for surface_config in ["8888", "565"]
    for mode in ["codec", "codec-zero-init", "scanline"]
    for dst_color_type in ["get-from-canvas"] + (
        # Skip uninteresting combinations.
        ["force-nonnative-premul-color"] if surface_config != "565" else []
    )
]

def png_codec_tests(name):
    """Generates various cc_test_with_flags targets for png_codec.cpp.

    Args:
        name: The name of the test_suite to generate.
    """

    all_tests = []

    for images, surface_config, mode, dst_color_type, dst_alpha_type in _TESTS:
        test_name = "png_codec_%s_%s_%s_%s_%s_test" % (
            images.replace("@skimage//:dm_pngs_", "").replace("_", "-"),
            surface_config,
            mode,
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
                "--pngCodecMode",
                mode,
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
    greppable_test_list = [
        "png_codec_gray8-opaque_8888_codec_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_8888_codec_force-grayscale_premul_test",
        "png_codec_gray8-opaque_8888_codec_force-nonnative-premul-color_premul_test",
        "png_codec_gray8-opaque_8888_codec-zero-init_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_8888_codec-zero-init_force-grayscale_premul_test",
        "png_codec_gray8-opaque_8888_codec-zero-init_force-nonnative-premul-color_premul_test",
        "png_codec_gray8-opaque_8888_scanline_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_8888_scanline_force-grayscale_premul_test",
        "png_codec_gray8-opaque_8888_scanline_force-nonnative-premul-color_premul_test",
        "png_codec_gray8-opaque_565_codec_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_565_codec-zero-init_get-from-canvas_premul_test",
        "png_codec_gray8-opaque_565_scanline_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_codec_get-from-canvas_unpremul_test",
        "png_codec_rgba8888-translucent_8888_codec_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_codec_force-nonnative-premul-color_unpremul_test",
        "png_codec_rgba8888-translucent_8888_codec_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-translucent_8888_codec-zero-init_get-from-canvas_unpremul_test",
        "png_codec_rgba8888-translucent_8888_codec-zero-init_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_codec-zero-init_force-nonnative-premul-color_unpremul_test",
        "png_codec_rgba8888-translucent_8888_codec-zero-init_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-translucent_8888_scanline_get-from-canvas_unpremul_test",
        "png_codec_rgba8888-translucent_8888_scanline_get-from-canvas_premul_test",
        "png_codec_rgba8888-translucent_8888_scanline_force-nonnative-premul-color_unpremul_test",
        "png_codec_rgba8888-translucent_8888_scanline_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_8888_codec_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_8888_codec_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_8888_codec-zero-init_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_8888_codec-zero-init_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_8888_scanline_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_8888_scanline_force-nonnative-premul-color_premul_test",
        "png_codec_rgba8888-opaque_565_codec_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_565_codec-zero-init_get-from-canvas_premul_test",
        "png_codec_rgba8888-opaque_565_scanline_get-from-canvas_premul_test",
    ]
    if greppable_test_list != all_tests:
        msg = [
            "Variable greppable_test_list is out of date. Please update it as follows:",
            "",
            "    greppable_test_list = [",
        ] + [
            "        \"" + test + "\","
            for test in all_tests
        ] + [
            "    ]",
        ]
        fail("\n".join(msg))
