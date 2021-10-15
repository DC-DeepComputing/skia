load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl", "tool_path")

def _clang_impl(ctx):
    print("HELLO WORLD")
    clang_binaries = ctx.attr.binary_locations.label.workspace_root
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "clang_trampoline.sh",
        ),
        tool_path(
            name = "ld",
            path = "lld_trampoline.sh",
        ),
        tool_path(
            name = "ar",
            path = "ar_trampline.sh",
        ),
        tool_path(
            name = "cpp",
            path = "/bin/false",
        ),
        tool_path(
            name = "gcov",
            path = "/bin/false",
        ),
        tool_path(
            name = "nm",
            path = "/bin/false",
        ),
        tool_path(
            name = "objdump",
            path = "/bin/false",
        ),
        tool_path(
            name = "strip",
            path = "/bin/false",
        ),
    ]

    # https://docs.bazel.build/versions/main/skylark/lib/cc_common.html#create_cc_toolchain_config_info
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        toolchain_identifier = "clang-toolchain",
        cxx_builtin_include_directories = [],
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "k8",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
        builtin_sysroot = "EGGS",
    )

provide_clang_toolchain_config = rule(
    attrs = {
        "binary_locations": attr.label(mandatory = True),
    },
    provides = [CcToolchainConfigInfo],
    implementation = _clang_impl,
)
