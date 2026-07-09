"""
This file specifies a clang toolchain that can run on a Windows host.

See download_windows_toolchain.bzl for more details on the creation of the toolchain.

It uses the usr subfolder of the built toolchain as a sysroot

It follows the example of:
 - linux_amd64_toolchain_config.bzl
"""

# https://github.com/bazelbuild/bazel/blob/master/tools/build_defs/cc/action_names.bzl
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

# https://github.com/bazelbuild/bazel/blob/master/tools/cpp/cc_toolchain_config_lib.bzl
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "action_config",
    "artifact_name_pattern",
    "feature",
    "flag_group",
    "flag_set",
    "tool",
    "variable_with_value",
    "with_feature_set",
)
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")

# These variables were copied from the generated @clang_windows_amd64//:vars.bzl file.
# They are available to be directly used here, but in order to do so, Bazel needs access
# to both the Clang and MSVC archives, even when we aren't going to use this toolchain.
# Due to the time required to download and extract these archives, we've opted to hard-code
# the versions and paths here.
MSVC_VERSION = "14.51.36231"
MSVC_INCLUDE = "VC/Tools/MSVC/" + MSVC_VERSION + "/include"
MSVC_LIB = "VC/Tools/MSVC/" + MSVC_VERSION + "/lib"

WIN_SDK_VERSION = "10.0.26100.0"
WIN_SDK_INCLUDE = "win_sdk/Include/" + WIN_SDK_VERSION
WIN_SDK_LIB = "win_sdk/Lib/" + WIN_SDK_VERSION

def _windows_amd64_toolchain_info(ctx):
    # https://bazel.build/rules/lib/builtins/Label#repo_name
    clang_repo_name = ctx.attr.clang_windows_amd64.label.repo_name
    clang_toolchain_path = "external/" + clang_repo_name
    action_configs = _make_action_configs(clang_toolchain_path)
    features = [
        feature(
            name = "archive_param_file",
            enabled = True,
        ),
        feature(
            name = "compiler_param_file",
            enabled = True,
        ),
        feature(
            name = "linker_param_file",
            enabled = True,
        ),
        # This feature is explicitly blacklisted (unsupported) by rules_rust by definition.
        # Defining it as enabled by default means it is active for standard C++ compiles
        # but is guaranteed to be disabled when rules_rust queries the toolchain.
        # This allows us to select clang.exe for C++ and fallback to lld-link.exe for Rust.
        feature(
            name = "rules_rust_unsupported_feature",
            enabled = True,
        ),
    ]
    features += _make_default_flags(clang_toolchain_path)
    features += _make_diagnostic_flags()

    # Tells bazel that our compiler will add .exe etc to the outputs
    # https://stackoverflow.com/a/63889676
    artifact_name_patterns = [
        artifact_name_pattern(
            category_name = "executable",
            prefix = "",
            extension = ".exe",
        ),
        artifact_name_pattern(
            category_name = "dynamic_library",
            prefix = "",
            extension = ".dll",
        ),
        artifact_name_pattern(
            category_name = "static_library",
            prefix = "",
            extension = ".lib",
        ),
    ]

    # https://bazel.build/rules/lib/cc_common#create_cc_toolchain_config_info
    # Note, this rule is defined in Java code, not Starlark
    # https://cs.opensource.google/bazel/bazel/+/master:src/main/java/com/google/devtools/build/lib/starlarkbuildapi/cpp/CcModuleApi.java
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        action_configs = action_configs,
        # These are required, but do nothing
        compiler = "",
        target_cpu = "",
        target_libc = "",
        target_system_name = "",
        toolchain_identifier = "",
        artifact_name_patterns = artifact_name_patterns,
    )

provide_windows_amd64_toolchain_config = rule(
    attrs = {
        "clang_windows_amd64": attr.label(default = "@clang_windows_amd64//:compile_files"),
    },
    provides = [CcToolchainConfigInfo],
    implementation = _windows_amd64_toolchain_info,
)

def _make_action_configs(clang_toolchain_path):
    """
    This function sets up the tools needed to perform the various compile/link actions.

    By using ../external/... relative paths pointing directly to clang.exe and lld-link.exe,
    we bypass batch script trampolines. This allows direct CreateProcessW execution which
    natively supports forward-slash path separators (critical for rules_rust/rustc linking).
    """

    # https://cs.opensource.google/bazel/bazel/+/master:tools/cpp/cc_toolchain_config_lib.bzl;l=435;drc=3b9e6f201a9a3465720aad8712ab7bcdeaf2e5da
    clang_tool = tool(path = "../" + clang_toolchain_path + "/bin/clang.exe")
    clang_cl_tool = tool(path = "../" + clang_toolchain_path + "/bin/clang-cl.exe")
    link_tool = tool(path = "../" + clang_toolchain_path + "/bin/llvm-lib.bat")

    # https://cs.opensource.google/bazel/bazel/+/master:tools/cpp/cc_toolchain_config_lib.bzl;l=488;drc=3b9e6f201a9a3465720aad8712ab7bcdeaf2e5da
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
        tools = [
            tool(
                path = "../" + clang_toolchain_path + "/bin/clang-cl.exe",
                with_features = [with_feature_set(features = ["rules_rust_unsupported_feature"])],
            ),
            tool(path = "../" + clang_toolchain_path + "/bin/lld-link.exe"),
        ],
    )
    cpp_link_executable_action = action_config(
        action_name = ACTION_NAMES.cpp_link_executable,
        # Bazel assumes it is talking to clang when building an executable. There are
        # "-Wl" flags on the command: https://releases.llvm.org/6.0.1/tools/clang/docs/ClangCommandLineReference.html#cmdoption-clang-Wl
        tools = [
            tool(
                path = "../" + clang_toolchain_path + "/bin/clang-cl.exe",
                with_features = [with_feature_set(features = ["rules_rust_unsupported_feature"])],
            ),
            tool(path = "../" + clang_toolchain_path + "/bin/lld-link.exe"),
        ],
    )
    cpp_link_nodeps_dynamic_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        tools = [
            tool(
                path = "../" + clang_toolchain_path + "/bin/clang-cl.exe",
                with_features = [with_feature_set(features = ["rules_rust_unsupported_feature"])],
            ),
            tool(path = "../" + clang_toolchain_path + "/bin/lld-link.exe"),
        ],
    )

    # By default, there are no flags or libraries passed to the llvm-link tool, so
    # we need to specify them. The variables mentioned by expand_if_available are defined
    # https://bazel.build/docs/cc-toolchain-config-reference#cctoolchainconfiginfo-build-variables
    cpp_link_static_library_action = action_config(
        action_name = ACTION_NAMES.cpp_link_static_library,
        implies = ["archive_param_file"],
        flag_sets = [
            flag_set(
                flag_groups = [
                    flag_group(
                        # LLD in /lib mode expects the MSVC-style /OUT:<file> option
                        flags = ["/OUT:%{output_execpath}"],
                        # Despite the name, output_execpath just refers to linker output,
                        # e.g. libFoo.lib
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
        tools = [link_tool],
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

def _make_default_flags(clang_toolchain):
    """Here we define the flags for certain actions that are always applied.

    For any flag that might be conditionally applied, it should be defined in //bazel/copts.bzl.

    Flags that are set here will be unconditionally applied to everything we compile with
    this toolchain, even third_party deps.
    """
    full_msvc_include = clang_toolchain + "/" + MSVC_INCLUDE
    full_msvc_lib = clang_toolchain + "/" + MSVC_LIB
    full_win_sdk_include = clang_toolchain + "/" + WIN_SDK_INCLUDE
    full_win_sdk_lib = clang_toolchain + "/" + WIN_SDK_LIB

    # Note: These values must be kept in sync with those defined in cmake_exporter.go.
    cxx_compile_includes = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    # THIS ORDER MATTERS GREATLY. If these are in the wrong order, the
                    # #include_next directives will fail to find the files, causing a compilation
                    # error (or, without -no-canonical-prefixes, a mysterious case where files
                    # are included with an absolute path and fail the build).
                    "-isystem",
                    clang_toolchain + "/include/clang",
                    "-isystem",
                    clang_toolchain + "/include/clang-c",
                    "-isystem",
                    clang_toolchain + "/include/clang-tidy",
                    "-isystem",
                    clang_toolchain + "/include/lld",
                    "-isystem",
                    clang_toolchain + "/include/lldb",
                    "-isystem",
                    clang_toolchain + "/include/llvm",
                    "-isystem",
                    clang_toolchain + "/include/llvm-c",
                    "-isystem",
                    clang_toolchain + "/lib/clang/23/include",
                    "-isystem",
                    full_win_sdk_include + "/shared",
                    "-isystem",
                    full_win_sdk_include + "/ucrt",
                    "-isystem",
                    full_win_sdk_include + "/um",
                    "-isystem",
                    full_win_sdk_include + "/winrt",
                    "-isystem",
                    full_win_sdk_include + "/cppwinrt",
                    "-isystem",
                    full_msvc_include,
                    # We do not want clang to search in absolute paths for files. This makes
                    # Bazel think we are using an outside resource and fail the compile.
                    "-no-canonical-prefixes",
                ],
            ),
        ],
    )

    cpp_compile_flags = flag_set(
        actions = [
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "-std=c++20",
                    "-DNOMINMAX",  # https://stackoverflow.com/a/22744273
                ],
            ),
        ],
    )

    # THE HYBRID C++ COMPILATION-LINKING PIPELINE
    # -------------------------------------------
    # Our Windows C++ toolchain is designed as a custom hybrid pipeline:
    # 1. COMPILATION: Handled via 'clang.exe' in GCC driver mode. This allows Skia to perfectly
    #    preserve and parse its existing, highly granular Unix/GCC compilation copts and warning
    #    suppressions (configured in include/config/copts.bzl) without rewriting them.
    # 2. LINKING: Handled via 'clang-cl.exe' in MSVC driver mode. This allows C++ links to process
    #    standard positional Windows system libraries (like user32.lib, gdi32.lib, dxgi.lib) and
    #    resolve MSVC standard library and SDK search paths natively.
    #
    # THE LINKER SPLITTING MECHANISM:
    # -------------------------------
    # C++ links must run via 'clang-cl.exe' (MSVC driver) to parse search paths and positional libs.
    # Conversely, Rust targets ('rules_rust') emit raw MSVC command lines and MUST link directly
    # with the raw 'lld-link.exe' linker (rejecting compiler drivers like clang-cl).
    # We solve this natively in Bazel by declaring "rules_rust_unsupported_feature" as a default-enabled
    # toolchain feature. Because rules_rust blacklists this feature, it is guaranteed to be enabled
    # only for C++ links (selecting clang-cl.exe) and disabled for Rust links (selecting lld-link.exe).
    #
    # THE STATIC CRT (/MT) ALIGNMENT:
    # -------------------------------
    # When 'clang.exe' compiles C++ files in GCC compatibility mode on Windows, the resulting
    # object files natively target the Static C Runtime (/MT) by default.
    # To prevent severe dynamic vs static CRT linkage conflicts, our C++ link actions must also
    # target the Static CRT using '/MT' and explicitly load the corresponding static standard libraries.
    #
    # DETAILED BREAKDOWN OF SYSTEM AND CRT LIBRARIES:
    # ----------------------------------------------
    # Because GCC-mode compilations do not automatically inject MSVC-style '/DEFAULTLIB' directives
    # into C++ object files, the LLD-link linker does not automatically know which MSVC standard and
    # SDK libraries to load. We must list them explicitly after '/link':
    #
    # - libcpmt.lib: The standard MSVC Static C++ Standard Library (locale, cout, threads, vector).
    # - libcmt.lib: The MSVC Static C Runtime. Even though 'operator new' and 'operator delete'
    #   are C++ language constructs, MSVC compiles their raw physical memory allocation backends
    #   directly into libcmt.lib.
    # - libvcruntime.lib: Static VC Runtime Helper containing physical definitions for compiler
    #   intrinsics like 'memcpy' and 'memset', RTTI metadata, and exception handlers (_CxxThrowException).
    # - libucrt.lib: Modern Universal CRT containing basic string and math library functions.
    # - kernel32.lib: The core Win32 system library containing fundamental OS APIs.
    # - ntdll.lib: Native Windows NT system calls. Essential for resolving NtReadFile, NtWriteFile,
    #   and RtlNtStatusToDosError used heavily by Rust's standard library (std) OS abstractions!
    # - dxguid.lib: Physical GUID definition constants for DirectX/DXGI (required by Google Dawn
    #   to resolve D3D debugging GUIDs like WKPDID_D3DDebugObjectName).
    # - onecore.lib: The modern unified Windows runtime SDK umbrella library. Explicitly used to
    #   resolve modern secure OS handle comparisons (CompareObjectHandles) partitioned inside the SDK.
    link_exe_flags_cpp = flag_set(
        actions = [
            ACTION_NAMES.cpp_link_executable,
            ACTION_NAMES.cpp_link_dynamic_library,
            ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        ],
        with_features = [with_feature_set(features = ["rules_rust_unsupported_feature"])],
        flag_groups = [
            flag_group(
                flags = [
                    "/MT",
                    "/link",
                    "/subsystem:console",
                    "libcpmt.lib",
                    "libcmt.lib",
                    "libvcruntime.lib",
                    "libucrt.lib",
                    "kernel32.lib",
                    "ntdll.lib",
                    "dxguid.lib",
                    "onecore.lib",
                    "/LIBPATH:" + full_msvc_lib + "/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/ucrt/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/um/x64",
                ],
            ),
        ],
    )

    link_exe_flags_rust = flag_set(
        actions = [
            ACTION_NAMES.cpp_link_executable,
            ACTION_NAMES.cpp_link_dynamic_library,
            ACTION_NAMES.cpp_link_nodeps_dynamic_library,
        ],
        with_features = [with_feature_set(not_features = ["rules_rust_unsupported_feature"])],
        flag_groups = [
            flag_group(
                flags = [
                    "/LIBPATH:" + full_msvc_lib + "/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/ucrt/x64",
                    "/LIBPATH:" + full_win_sdk_lib + "/um/x64",
                ],
            ),
        ],
    )
    return [feature(
        "default_flags",
        enabled = True,
        flag_sets = [
            cxx_compile_includes,
            cpp_compile_flags,
            link_exe_flags_cpp,
            link_exe_flags_rust,
        ],
    )]

def _make_diagnostic_flags():
    """Here we define the flags that can be turned on via features to yield debug info."""
    cxx_diagnostic = flag_set(
        actions = [
            ACTION_NAMES.c_compile,
            ACTION_NAMES.cpp_compile,
        ],
        flag_groups = [
            flag_group(
                flags = [
                    "--trace-includes",
                    "-v",
                ],
            ),
        ],
    )

    link_diagnostic = flag_set(
        actions = [ACTION_NAMES.cpp_link_executable],
        flag_groups = [
            flag_group(
                flags = [
                    "-Wl,-verbose",
                    "-v",
                ],
            ),
        ],
    )

    link_search_dirs = flag_set(
        actions = [ACTION_NAMES.cpp_link_executable],
        flag_groups = [
            flag_group(
                flags = [
                    "--print-search-dirs",
                ],
            ),
        ],
    )
    return [
        # Running a Bazel command with --features diagnostic will cause the compilation and
        # link steps to be more verbose.
        feature(
            "diagnostic",
            enabled = False,
            flag_sets = [
                cxx_diagnostic,
                link_diagnostic,
            ],
        ),
        feature(
            "diagnostic_link",
            enabled = False,
            flag_sets = [
                link_diagnostic,
            ],
        ),
        # Running a Bazel command with --features print_search_dirs will cause the link to fail
        # but directories searched for libraries, etc will be displayed.
        feature(
            "print_search_dirs",
            enabled = False,
            flag_sets = [
                link_search_dirs,
            ],
        ),
    ]
