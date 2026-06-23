# Custom repository rule to download NDK r29 and generate the toolchain targets dynamically
def _ndk_repository_impl(ctx):
    # 1. Detect the host operating system
    os_name = ctx.os.name.lower()
    if "linux" in os_name:
        host_os = "linux"
        url = "https://dl.google.com/android/repository/android-ndk-r29-linux.zip"
        prebuilt_dir = "toolchains/llvm/prebuilt/linux-x86_64"
    elif "mac os x" in os_name or "darwin" in os_name:
        host_os = "darwin"
        url = "https://dl.google.com/android/repository/android-ndk-r29-darwin.zip"
        prebuilt_dir = "toolchains/llvm/prebuilt/darwin-x86_64"
    elif "windows" in os_name:
        host_os = "windows"
        url = "https://dl.google.com/android/repository/android-ndk-r29-windows.zip"
        prebuilt_dir = "toolchains/llvm/prebuilt/windows-x86_64"
    else:
        fail("❌ [Skia Hermetic NDK] Unsupported host operating system: %s" % ctx.os.name)

    # 2. Download and extract the correct NDK zip hermetically
    print("[Skia Hermetic NDK] Host OS detected: %s" % host_os.upper())
    print("[Skia Hermetic NDK] Downloading Android NDK r29 hermetically...")
    ctx.download_and_extract(
        url = url,
        stripPrefix = "android-ndk-r29",
    )

    # 3. Write a clean BUILD.bazel file defining the C++ toolchains natively inside the repo!
    ctx.file("BUILD.bazel", """
load("@rules_cc//cc:defs.bzl", "cc_toolchain")
load("@skia//bazel:ndk_cc_toolchain_config.bzl", "ndk_cc_toolchain_config")

package(default_visibility = ["//visibility:public"])

# Include all compilers and sysroot headers dynamically using glob under the LLVM prebuilt folder
filegroup(
    name = "all_files",
    srcs = glob([
        "{prebuilt_dir}/**",
    ]),
)

# 64-bit ARM Toolchain
ndk_cc_toolchain_config(
    name = "ndk_arm64_v8a_toolchain_config",
    cpu = "arm64-v8a",
    host_os = "{host_os}",
    prebuilt_dir = "{prebuilt_dir}",
)

cc_toolchain(
    name = "ndk_arm64_v8a",
    all_files = ":all_files",
    ar_files = ":all_files",
    compiler_files = ":all_files",
    dwp_files = ":all_files",
    linker_files = ":all_files",
    objcopy_files = ":all_files",
    strip_files = ":all_files",
    supports_param_files = False,
    toolchain_config = ":ndk_arm64_v8a_toolchain_config",
)

toolchain(
    name = "ndk_arm64_v8a_toolchain",
    exec_compatible_with = [
        "@platforms//os:{host_os_exec_constraint}",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:android",
        "@platforms//cpu:arm64",
    ],
    toolchain = ":ndk_arm64_v8a",
    toolchain_type = "@rules_cc//cc:toolchain_type",
)

# 32-bit ARM Toolchain
ndk_cc_toolchain_config(
    name = "ndk_armeabi_v7a_toolchain_config",
    cpu = "armeabi-v7a",
    host_os = "{host_os}",
    prebuilt_dir = "{prebuilt_dir}",
)

cc_toolchain(
    name = "ndk_armeabi_v7a",
    all_files = ":all_files",
    ar_files = ":all_files",
    compiler_files = ":all_files",
    dwp_files = ":all_files",
    linker_files = ":all_files",
    objcopy_files = ":all_files",
    strip_files = ":all_files",
    supports_param_files = False,
    toolchain_config = ":ndk_armeabi_v7a_toolchain_config",
)

toolchain(
    name = "ndk_armeabi_v7a_toolchain",
    exec_compatible_with = [
        "@platforms//os:{host_os_exec_constraint}",
        "@platforms//cpu:x86_64",
    ],
    target_compatible_with = [
        "@platforms//os:android",
        "@platforms//cpu:armv7",
    ],
    toolchain = ":ndk_armeabi_v7a",
    toolchain_type = "@rules_cc//cc:toolchain_type",
)
""".format(
        host_os = host_os,
        prebuilt_dir = prebuilt_dir,
        host_os_exec_constraint = "macos" if host_os == "darwin" else host_os,
    ))

ndk_repository = repository_rule(
    implementation = _ndk_repository_impl,
)

# Bzlmod module extension wrapper
def _ndk_ext_impl(ctx):
    ndk_repository(name = "ndk_linux_amd64")
    return None

ndk_ext = module_extension(
    implementation = _ndk_ext_impl,
)
