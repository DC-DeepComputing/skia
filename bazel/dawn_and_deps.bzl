"""
This module defines the dawn_and_deps repository rule and its Bzlmod extension.
"""

def _dawn_and_deps_impl(ctx):
    if ctx.attr.local_path:
        # If a local path is provided, use it instead of downloading.
        # This is much faster for iteration.
        # We use ctx.path to get an absolute path, then use execution to copy.
        local_path = ctx.path(ctx.attr.local_path)
        ctx.execute(["cp", "-rn", str(local_path) + "/.", "."])

        # We still need to copy the build files from the main repo
        ctx.file("cmake_external_build.bzl", ctx.read(ctx.path(Label("//bazel/external/dawn:cmake_external_build.bzl"))))
        ctx.file("build_dawn_bazel.py", ctx.read(ctx.path(Label("//bazel/external/dawn:build_dawn_bazel.py"))))
        ctx.file("BUILD.bazel", ctx.read(ctx.path(Label("//bazel/external/dawn:BUILD.bazel"))))
        return

    # Dependencies for Dawn and Tint as seen in DEPS and cmake_utils.py
    # Format: (name, remote, commit)
    deps = [
        ("dawn", "https://dawn.googlesource.com/dawn", "d2eac07e6a1abd38cf6d3d13d6d587b6c763b672"),
        ("abseil-cpp", "https://chromium.googlesource.com/chromium/src/third_party/abseil-cpp", "dd4432fb02583b7a5b2b4fbf2012f7053e286ce0"),
        ("jinja2", "https://chromium.googlesource.com/chromium/src/third_party/jinja2", "c3027d884967773057bf74b957e3fea87e5df4d7"),
        ("markupsafe", "https://chromium.googlesource.com/chromium/src/third_party/markupsafe", "4256084ae14175d38a3ff7d739dca83ae49ccec6"),
        ("glslang", "https://chromium.googlesource.com/external/github.com/KhronosGroup/glslang", "1d47ffa8ac4374a19b302021e216a20f22a3de92"),
        ("webgpu-headers", "https://chromium.googlesource.com/external/github.com/webgpu-native/webgpu-headers", "706853a9da45b8e89b7ea005aa267294d115f8ce"),
        ("egl-registry", "https://skia.googlesource.com/external/github.com/KhronosGroup/EGL-Registry", "b055c9b483e70ecd57b3cf7204db21f5a06f9ffe"),
        ("opengl-registry", "https://skia.googlesource.com/external/github.com/KhronosGroup/OpenGL-Registry", "14b80ebeab022b2c78f84a573f01028c96075553"),
        ("spirv-headers", "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Headers", "869266ad9e6050197d87cf0a22aab59abf7ad008"),
        ("spirv-tools", "https://skia.googlesource.com/external/github.com/KhronosGroup/SPIRV-Tools", "a52828d672ab1622c308d0a6fc99630b79561609"),
        ("vulkan-headers", "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Headers", "f6a6f7ab165cedbfa2a7d0c93fe27a2d01ce09c8"),
        ("vulkan-utility-libraries", "https://chromium.googlesource.com/external/github.com/KhronosGroup/Vulkan-Utility-Libraries", "5f62a4c3c686d3d3d1b3973a4397dd4d4768f3ad"),
        ("swiftshader", "https://swiftshader.googlesource.com/SwiftShader", "89556131bf9d48af3c5c9fbb9a3322e706da89a3"),
    ]

    for name, remote, commit in deps:
        if "googlesource.com" in remote:
            url = remote + "/+archive/" + commit + ".tar.gz"
        else:
            fail("Unsupported remote: " + remote)

        ctx.download_and_extract(
            url = url,
            output = name,
            type = "tar.gz",
        )

    # Download hermetic build tools directly into the repo
    # CMake
    ctx.download_and_extract(
        url = "https://chrome-infra-packages.appspot.com/dl/skia/bots/cmake_linux/+/version%3A5",
        output = "cmake",
        type = "zip",
        sha256 = "7d29996e77af9f5639565732790ae432fffd30249dc5036540738a32f97bd3c1",
    )

    # Ninja
    ctx.download_and_extract(
        url = "https://chrome-infra-packages.appspot.com/dl/infra/3pp/tools/ninja/linux-amd64/+/version%3A2@1.12.1.chromium.4",
        output = "ninja",
        type = "zip",
        sha256 = "3f1f1cc0f69a1bcfdf67fb6c2bc7419b1dd812bb8d0e79afa9bfa8a3553b5082",
    )

    # Copy build-related files from the main repository into the root of this repo.
    # This allows the BUILD.bazel file to use relative loads and find the script.
    ctx.file("cmake_external_build.bzl", ctx.read(ctx.path(Label("//bazel/external/dawn:cmake_external_build.bzl"))))
    ctx.file("build_dawn_bazel.py", ctx.read(ctx.path(Label("//bazel/external/dawn:build_dawn_bazel.py"))))
    ctx.file("BUILD.bazel", ctx.read(ctx.path(Label("//bazel/external/dawn:BUILD.bazel"))))

dawn_and_deps = repository_rule(
    implementation = _dawn_and_deps_impl,
    attrs = {
        "local_path": attr.string(
            doc = "Optional local path to a directory containing dawn and its deps.",
        ),
    },
)

def _dawn_and_deps_extension_impl(ctx):
    local_path = ctx.os.environ.get("SKIA_DAWN_SEED_DIR", "")
    dawn_and_deps(name = "dawn_and_deps", local_path = local_path)

dawn_and_deps_extension = module_extension(
    implementation = _dawn_and_deps_extension_impl,
    environ = ["SKIA_DAWN_SEED_DIR"],
)
