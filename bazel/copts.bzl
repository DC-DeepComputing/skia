CORE_COPTS = [
    "-fstrict-aliasing",
    "-fPIC",
    "-fvisibility=hidden",
    "-fno-exceptions",
    "-fno-rtti",  # Reduces code size
]

OPT_LEVEL = select({
    "//bazel/common_config_settings:debug_build": [
        "--optimize=0",
        "--debug",
    ],
    "//bazel/common_config_settings:release_build": [
        "--optimize=3",
    ],
    "//bazel/common_config_settings:fast_build": [
        "--optimize=0",
        "-gline-tables-only",
    ],
})

WARNINGS = [
    "-Wno-psabi",  # noisy
]

DEFAULT_COPTS = CORE_COPTS + OPT_LEVEL + WARNINGS
