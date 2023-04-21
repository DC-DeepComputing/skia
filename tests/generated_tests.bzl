"""GENERATED FILE, DO NOT MODIFY."""

load(
    ":tests.bzl",
    "CODEC",
    "GANESH",
    "PATHOPS",
    "PIXEL_5",
    "PIXEL_7",
    "skia_android_test",
)

def generated_tests(name):
    """Produces the generated Bazel test targets.

    Args:
        name: Unused, but required to appease Buildifier.
    """

    skia_android_test(
        name = "pixel5_codec_test",
        device = PIXEL_5,
        test_group = CODEC,
    )

    skia_android_test(
        name = "pixel5_pathops_test",
        device = PIXEL_5,
        test_group = PATHOPS,
    )

    skia_android_test(
        name = "pixel7_ganesh_test",
        device = PIXEL_7,
        test_group = GANESH,
    )
