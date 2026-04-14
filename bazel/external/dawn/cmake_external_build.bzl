"""
This module defines the cmake_external_build Starlark rule.
"""

load("@rules_cc//cc:action_names.bzl", "CPP_COMPILE_ACTION_NAME", "CPP_LINK_STATIC_LIBRARY_ACTION_NAME", "C_COMPILE_ACTION_NAME")
load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cc_toolchain")

def _cmake_external_build_impl(ctx):
    cc_toolchain = find_cc_toolchain(ctx)

    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
        requested_features = ctx.features,
        unsupported_features = ctx.disabled_features,
    )

    c_compiler_path = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = C_COMPILE_ACTION_NAME,
    )
    cpp_compiler_path = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = CPP_COMPILE_ACTION_NAME,
    )

    archiver_path = cc_common.get_tool_for_action(
        feature_configuration = feature_configuration,
        action_name = CPP_LINK_STATIC_LIBRARY_ACTION_NAME,
    )

    compile_variables = cc_common.create_compile_variables(
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        user_compile_flags = ctx.fragments.cpp.cxxopts + ctx.fragments.cpp.copts,
    )
    cxx_flags_list = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = CPP_COMPILE_ACTION_NAME,
        variables = compile_variables,
    )

    # Skip the first element which is the compiler path
    cxx_flags = cxx_flags_list[1:]

    output_lib = ctx.actions.declare_file(ctx.attr.output_lib_name)
    output_headers_dir = ctx.actions.declare_directory(ctx.attr.name + "_include")

    build_dir_path = ctx.bin_dir.path + "/_cmake_build_" + ctx.label.name

    args = ctx.actions.args()
    args.add("--bazel")
    args.add("--cc", c_compiler_path)
    args.add("--cxx", cpp_compiler_path)
    args.add("--ar", archiver_path)
    args.add("--cmake_path", ctx.file.cmake_tool.path)
    args.add("--ninja_path", ctx.file.ninja_tool.path)

    # The root of the @dawn repository within the execroot.
    repo_root = ctx.label.workspace_root if ctx.label.workspace_root else "."
    args.add("--repo_root", repo_root)
    args.add("--output_lib", output_lib.path)
    args.add("--output_headers", output_headers_dir.path)
    args.add("--build_dir", build_dir_path)

    for flag in cxx_flags:
        if flag:
            # Using = VALUE to prevent argparse from thinking it is a separate option
            args.add("--cxx_flags=" + flag)

    ctx.actions.run(
        outputs = [output_lib, output_headers_dir],
        inputs = depset(
            direct = [
                ctx.file.cmake_tool,
                ctx.file.ninja_tool,
            ],
            transitive = [ctx.attr.dawn_and_deps.files],
        ),
        executable = ctx.executable.build_script,
        arguments = [args],
        mnemonic = "CMakeBuild",
        progress_message = "Building Dawn/Tint with CMake (incremental)...",
        use_default_shell_env = True,
        execution_requirements = {
            "no-sandbox": "1",
            "local": "1",
        },
    )

    return [
        DefaultInfo(files = depset([output_lib, output_headers_dir])),
        CcInfo(
            compilation_context = cc_common.create_compilation_context(
                includes = depset([output_headers_dir.path]),
            ),
        ),
    ]

cmake_external_build = rule(
    implementation = _cmake_external_build_impl,
    attrs = {
        "build_script": attr.label(
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
        "dawn_and_deps": attr.label(
            allow_files = True,
        ),
        "cmake_tool": attr.label(
            allow_single_file = True,
        ),
        "ninja_tool": attr.label(
            allow_single_file = True,
        ),
        "output_lib_name": attr.string(mandatory = True),
        "_cc_toolchain": attr.label(
            default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
        ),
    },
    fragments = ["cpp"],
    toolchains = ["@bazel_tools//tools/cpp:toolchain_type"],
)
