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
    action_configs = make_action_configs()
    cxx_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    # THIS ORDER MATTERS GREATLY for glibc. If these are in the wrong order, the
                    # #include_next directives will walk off the end of the specified system
                    # folders here and look in the absolute path that happens to contain
                    # the clang executable. Because that looks like an absolute path to
                    # Bazel, it will declare the build "not well specified" and fail.
                    # musl does not seem to use #include_next as much, so this is less of an issue
                    "-isystem",
                    "external/clang_linux_amd64/include/c++/v1",
                    "-isystem",
                    "external/clang_linux_amd64/lib/clang/13.0.0/include",
                    "-isystem",
                    "external/clang_linux_amd64/usr/include/x86_64-linux-musl",
                    # The following lines can be useful for debugging if uncommented.
                    # "--trace-includes",
                    # "-v",
                ],
            ),
        ],
    )

    cpp_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "-std=c++14",
                    "-stdlib=libc++",
                    "-D_LIBCPP_PROVIDES_DEFAULT_RUNE_TABLE",
                    "-D_LIBCPP_HAS_MUSL_LIBC",
                ],
            ),
        ],
    )

    link_exe_flags = flag_set(
        actions = [ACTION_NAMES.cpp_link_executable],
        flag_groups = [
            flag_group(
                flags = [
                    # This is the path to libc.a and other libraries.
                    "-Lexternal/clang_linux_amd64/usr/lib/x86_64-linux-musl",
                    "-Lexternal/clang_linux_amd64/lib",
                    # We chose to use the llvm runtime, not the gcc one because it is already
                    # included in the clang binary
                    "--rtlib=compiler-rt",
                    "-static",
                    #"-Wl,-rpath,$ORIGIN",
                    # "-nostdinc++",
                    #"-nostdlib",
                    #"-nostdlib++",
                    "-std=c++14",
                    "-lc++abi",
                    "-lunwind",
                    "-lc++",
                    # The following lines can be useful for debugging if uncommented.
                    "-Wl,--verbose",
                    "-v",
                    # If the following is uncommented, nothing will be built, but the
                    # directories searched for libraries, etc will be displayed.
                    # "--print-search-dirs",
                ],
            ),
        ],
    )

    used_features = [
        feature(
            "some_feature",
            enabled = True,
            flag_sets = [
                cxx_compile_includes,
                cpp_compile_includes,
                link_exe_flags,
            ],
        ),
    ]

    # https://docs.bazel.build/versions/main/skylark/lib/cc_common.html#create_cc_toolchain_config_info
    # Note, this rule is defined in Java code, not Starlark
    # https://cs.opensource.google/bazel/bazel/+/master:src/main/java/com/google/devtools/build/lib/starlarkbuildapi/cpp/CcModuleApi.java
    foo = cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = used_features,
        action_configs = action_configs,
        toolchain_identifier = "clang-toolchain",
        cxx_builtin_include_directories = [
            "external/clang_linux_amd64/lib/clang/13.0.0/include",
            "clang_linux_amd64/lib/clang/13.0.0/include",
        ],
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "k8",
        target_libc = "musl",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        #cc_target_os = "linux",
        #builtin_sysroot = "external/clang_and_llvm_linux",
        #builtin_sysroot = "external/amd64_libc-cross/x86_64-linux-gnu",
        builtin_sysroot = "external/clang_linux_amd64/usr",
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

def make_action_configs():
    clang_tool = tool(path = "clang_trampoline.sh")
    lld_tool = tool(path = "lld_trampoline.sh")
    ar_tool = tool(path = "ar_trampoline.sh")

    assemble_action = action_config(
        action_name = ACTION_NAMES.assemble,
        tools = [clang_tool],
    )
    c_compile_action = action_config(
        action_name = ACTION_NAMES.c_compile,
        tools = [clang_tool],
    )
    cpp_compile_action = action_config(
        action_name = ACTION_NAMES.cpp_compile,
        tools = [clang_tool],
    )
    linkstamp_compile_action = action_config(
        action_name = ACTION_NAMES.linkstamp_compile,
        tools = [clang_tool],
    )
    preprocess_assemble_action = action_config(
        action_name = ACTION_NAMES.preprocess_assemble,
        tools = [clang_tool],
    )

    cpp_link_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_dynamic_library,
        tools = [lld_tool],
    )
    cpp_link_executable_action = action_config(
        action_name = ACTION_NAMES.cpp_link_executable,
        flag_sets = [
            flag_set(
                flag_groups = [
                    flag_group(
                        flags = ["-fuse-ld=lld"],
                    ),
                ],
            ),
        ],
        # Bazel assumes it is talking to clang when building an executable. There are
        # "-Wl" flags on the command: https://releases.llvm.org/6.0.1/tools/clang/docs/ClangCommandLineReference.html#cmdoption-clang-Wl
        tools = [clang_tool],
    )
    cpp_link_nodeps_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        tools = [lld_tool],
    )

    # This is the same rule as
    # https://github.com/emscripten-core/emsdk/blob/7f39d100d8cd207094decea907121df72065517e/bazel/emscripten_toolchain/crosstool.bzl#L143
    # By default, there are no flags or libraries passed to the llvm-ar tool, so
    # we need to specify them. The variables mentioned by expand_if_available are defined
    # https://docs.bazel.build/versions/main/cc-toolchain-config-reference.html#cctoolchainconfiginfo-build-variables
    cpp_link_static_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_static_library,
        flag_sets = [
            flag_set(
                flag_groups = [
                    flag_group(
                        # https://llvm.org/docs/CommandGuide/llvm-ar.html
                        # replace existing files or insert them if they already exist,
                        # create the file if it doesn't already exist
                        # symbol table should be added
                        # Deterministic timestamps should be used
                        flags = ["rcsD", "%{output_execpath}"],
                        # Despite the name, output_execpath just refers to linker output,
                        # e.g. libFoo.a
                        expand_if_available = "output_execpath",
                    ),
                ],
            ),
            flag_set(
                flag_groups = [
                    flag_group(
                        iterate_over = "libraries_to_link",
                        flag_groups = [
                            flag_group(
                                flags = ["%{libraries_to_link.name}"],
                                expand_if_equal = variable_with_value(
                                    name = "libraries_to_link.type",
                                    value = "object_file",
                                ),
                            ),
                            flag_group(
                                flags = ["%{libraries_to_link.object_files}"],
                                iterate_over = "libraries_to_link.object_files",
                                expand_if_equal = variable_with_value(
                                    name = "libraries_to_link.type",
                                    value = "object_file_group",
                                ),
                            ),
                        ],
                        expand_if_available = "libraries_to_link",
                    ),
                ],
            ),
            flag_set(
                flag_groups = [
                    flag_group(
                        flags = ["@%{linker_param_file}"],
                        expand_if_available = "linker_param_file",
                    ),
                ],
            ),
        ],
        tools = [ar_tool],
    )

    action_configs = [
        assemble_action,
        c_compile_action,
        cpp_compile_action,
        cpp_link_dynamic_library_action,
        cpp_link_executable_action,
        cpp_link_nodeps_dynamic_library_action,
        cpp_link_static_library_action,
        linkstamp_compile_action,
        preprocess_assemble_action,
    ]
    return action_configs
