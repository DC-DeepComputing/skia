"""GENERATED FILE, DO NOT MODIFY."""

load(
    ":android.bzl",
    "CODEC",
    "DEBUG",
    "GANESH",
    "GL",
    "PIXEL_5",
    "PIXEL_7",
    "RELEASE",
    "VULKAN",
    "skia_android_unit_test",
)

def generated_tests(name):
    """Produces a set of Bazel targets generated from CI task names in //infra/bots/jobs.json.

    Args:
        name: Unused, but required to appease Buildifier.
    """

    skia_android_unit_test(
        name = "pixel5_codec_vulkan_release_test",
        device = PIXEL_5,
        test_group = CODEC,
        flavors = [
            RELEASE,
            VULKAN,
        ],
    )

    skia_android_unit_test(
        name = "pixel7_ganesh_gl_debug_test",
        device = PIXEL_7,
        test_group = GANESH,
        flavors = [
            DEBUG,
            GL,
        ],
    )
