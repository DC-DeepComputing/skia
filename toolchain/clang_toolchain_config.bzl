load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "action_config",
    "env_entry",
    "env_set",
    "feature",
    "feature_set",
    "flag_group",
    "flag_set",
    "tool",
    "tool_path",
    "variable_with_value",
    "with_feature_set",
)
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

def _clang_impl(ctx):
    print("HELLO WORLD")
    print(ctx.bin_dir.path)
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
            path = "ar_trampoline.sh",
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

    sysroot_includes = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
            ACTION_NAMES.cpp_header_parsing,
            ACTION_NAMES.cpp_module_compile,
            ACTION_NAMES.cpp_module_codegen,
            ACTION_NAMES.lto_backend,
            ACTION_NAMES.clif_match,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    #                    "--trace-includes",
                    #                    "-v",
                    # THIS ORDER MATTERS GREATLY. If these are in the wrong order, the
                    # #include_next directives will walk off the end of the specified system
                    # folders here and look in the absolute path that happens to contain
                    # the clang executable. Because that looks like an absolute path to
                    # Bazel, it will declare the build "not well specified" and fail.
                    "-isystem",
                    "external/clang_and_llvm_linux/lib/clang/13.0.0/include",
                    "-isystem",
                    "external/linux_libc/include",
                    "-isystem",
                    "external/linux_libc/include/x86_64-linux-gnu",  # provides <asm/*>
                    "-isystem",
                    "external/amd64_libc-cross/x86_64-linux-gnu/include",
                    "-isystem",
                    "external/clang_and_llvm_linux/include/c++/v1",
                ],
            ),
        ],
    )

    used_features = [
        feature("some_feature", enabled = True, flag_sets = [
            sysroot_includes,
        ]),
    ]

    # https://docs.bazel.build/versions/main/skylark/lib/cc_common.html#create_cc_toolchain_config_info
    foo = cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = used_features,
        toolchain_identifier = "clang-toolchain",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "k8",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
        builtin_sysroot = "external/clang_and_llvm_linux",
    )

    # print(foo)
    return foo

provide_clang_toolchain_config = rule(
    attrs = {
        "binary_locations": attr.label(mandatory = True),
    },
    provides = [CcToolchainConfigInfo],
    implementation = _clang_impl,
)
