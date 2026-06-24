"""Bzlmod extension and repository rule to download the Android NDK hermetically and configure its toolchains."""

# Android NDK configuration variables for easy upgrades in the future
NDK_VERSION = "r29"
NDK_API_LEVEL = 21
NDK_STRIP_PREFIX = "android-ndk-r29"

def _hybrid_ndk_repository_impl(ctx):
    # Step 1: Detect host OS and download the correct NDK archive
    os_name = ctx.os.name.lower()
    is_windows = False
    executable_extension = ""

    if "linux" in os_name:
        url = "https://dl.google.com/android/repository/android-ndk-%s-linux.zip" % NDK_VERSION
        clang_directory = "toolchains/llvm/prebuilt/linux-x86_64"
    elif "mac os x" in os_name or "darwin" in os_name:
        url = "https://dl.google.com/android/repository/android-ndk-%s-darwin.zip" % NDK_VERSION
        clang_directory = "toolchains/llvm/prebuilt/darwin-x86_64"
    elif "windows" in os_name:
        url = "https://dl.google.com/android/repository/android-ndk-%s-windows.zip" % NDK_VERSION
        clang_directory = "toolchains/llvm/prebuilt/windows-x86_64"
        is_windows = True
        executable_extension = ".exe"
    else:
        fail("Unsupported OS for hermetic NDK download: %s" % os_name)

    # Step 2: Download and extract NDK hermetically directly into the repository root!
    ctx.download_and_extract(
        url = url,
        stripPrefix = NDK_STRIP_PREFIX,
    )

    # Create the symlink hack expected by @rules_android_ndk templates
    ctx.symlink("sources", "ndk/sources")

    sysroot_directory = "%s/sysroot" % clang_directory

    # Step 3: Run the downloaded clang to find its resource directory
    clang_path = "%s/bin/clang%s" % (clang_directory, executable_extension)
    result = ctx.execute([clang_path, "--print-resource-dir"])
    if result.return_code != 0:
        fail("Failed to execute downloaded clang: %s" % result.stderr)

    stdout = result.stdout.strip()
    if is_windows:
        stdout = stdout.replace("\\", "/")

    # Extract the relative resource directory path
    clang_resource_directory = stdout.split(clang_directory)[1].strip("/")

    # Step 4: Write the official BUILD files using the @rules_android_ndk templates
    repository_name = ctx.attr._build.workspace_name
    api_level = ctx.attr.api_level

    # A: Write root BUILD.bazel
    ctx.template(
        "BUILD.bazel",
        ctx.attr._template_ndk_root,
        {
            "{clang_directory}": clang_directory,
        },
        executable = False,
    )

    # B: Write target_systems.bzl
    ctx.template(
        "target_systems.bzl",
        ctx.attr._template_target_systems,
        {},
        executable = False,
    )

    # C: Write toolchains/llvm/prebuilt/<host>/BUILD.bazel
    ctx.template(
        "%s/BUILD.bazel" % clang_directory,
        ctx.attr._template_ndk_clang,
        {
            "{repository_name}": repository_name,
            "{api_level}": str(api_level),
            "{clang_resource_directory}": clang_resource_directory,
            "{sysroot_directory}": sysroot_directory,
            "{executable_extension}": executable_extension,
        },
        executable = False,
    )

    # D: Write toolchains/llvm/prebuilt/<host>/sysroot/BUILD.bazel
    ctx.template(
        "%s/BUILD.bazel" % sysroot_directory,
        ctx.attr._template_ndk_sysroot,
        {
            "{api_level}": str(api_level),
        },
        executable = False,
    )

# Define our custom hybrid repository rule
hybrid_ndk_repository = repository_rule(
    attrs = {
        "api_level": attr.int(default = NDK_API_LEVEL),
        "_build": attr.label(default = "@rules_android_ndk//:BUILD", allow_single_file = True),
        "_template_ndk_root": attr.label(default = "@rules_android_ndk//:BUILD.ndk_root.tpl", allow_single_file = True),
        "_template_target_systems": attr.label(default = "@rules_android_ndk//:target_systems.bzl.tpl", allow_single_file = True),
        "_template_ndk_clang": attr.label(default = "@rules_android_ndk//:BUILD.ndk_clang.tpl", allow_single_file = True),
        "_template_ndk_sysroot": attr.label(default = "@rules_android_ndk//:BUILD.ndk_sysroot.tpl", allow_single_file = True),
    },
    implementation = _hybrid_ndk_repository_impl,
    doc = "Hermetic repository rule that downloads the NDK and generates standard-compliant Bazel toolchains in-place.",
)

def _ndk_ext_impl(module_ctx):
    """Bzlmod extension that instantiates our hybrid repository rule."""
    hybrid_ndk_repository(
        name = "androidndk",
        api_level = NDK_API_LEVEL,
    )

ndk_ext = module_extension(
    implementation = _ndk_ext_impl,
)
