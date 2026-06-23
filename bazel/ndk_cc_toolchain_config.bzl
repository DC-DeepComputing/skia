load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "flag_group",
    "flag_set",
    "tool_path",
)
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")

# Supported architectures
_ARMEABI_V7A = "armeabi-v7a"
_ARM64_V8A = "arm64-v8a"

_all_compile_actions = [
    ACTION_NAMES.c_compile,
    ACTION_NAMES.cpp_compile,
    ACTION_NAMES.linkstamp_compile,
    ACTION_NAMES.assemble,
    ACTION_NAMES.preprocess_assemble,
    ACTION_NAMES.cpp_header_parsing,
    ACTION_NAMES.cpp_module_compile,
    ACTION_NAMES.cpp_module_codegen,
    ACTION_NAMES.clif_match,
    ACTION_NAMES.lto_backend,
]

_all_link_actions = [
    ACTION_NAMES.cpp_link_executable,
    ACTION_NAMES.cpp_link_dynamic_library,
    ACTION_NAMES.cpp_link_nodeps_dynamic_library,
]

def _get_compile_flags(cpu, workspace_name, sysroot_prefix):
    sysroot_path = "external/" + workspace_name + "/" + sysroot_prefix
    
    flags = [
        "--sysroot=" + sysroot_path,
        "-stdlib=libc++",
        "-isystem", sysroot_path + "/usr/include/c++/v1",
        "-isystem", sysroot_path + "/usr/include",
        "-fpic",
        "-no-canonical-prefixes",
        "-Wno-invalid-command-line-argument",
        "-Wno-unused-command-line-argument",
        "-funwind-tables",
        "-fstack-protector-strong",
        "-fno-addrsig",
    ]
    if cpu == _ARMEABI_V7A:
        flags += [
            "-target", "armv7a-linux-androideabi29",
            "-march=armv7-a",
            "-mfloat-abi=softfp",
            "-mfpu=vfpv3-d16",
            "-isystem", sysroot_path + "/usr/include/arm-linux-androideabi",
        ]
    elif cpu == _ARM64_V8A:
        flags += [
            "-target", "aarch64-linux-android29",
            "-isystem", sysroot_path + "/usr/include/aarch64-linux-android",
        ]
    return flags

def _get_link_flags(cpu, workspace_name, sysroot_prefix):
    sysroot_path = "external/" + workspace_name + "/" + sysroot_prefix
    
    flags = [
        "--sysroot=" + sysroot_path,
        "-stdlib=libc++",
        "-no-canonical-prefixes",
        "-Wl,-z,relro",
        "-lm",
    ]
    if cpu == _ARMEABI_V7A:
        flags += [
            "-target", "armv7a-linux-androideabi29",
        ]
    elif cpu == _ARM64_V8A:
        flags += [
            "-target", "aarch64-linux-android29",
        ]
    return flags

def _ndk_cc_toolchain_config_impl(ctx):
    cpu = ctx.attr.cpu
    host_os = ctx.attr.host_os
    prebuilt_dir = ctx.attr.prebuilt_dir
    workspace_name = ctx.label.workspace_name
    
    sysroot_prefix = prebuilt_dir + "/sysroot"
    
    features = [
        feature(
            name = "default_compile_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _all_compile_actions,
                    flag_groups = [flag_group(flags = _get_compile_flags(cpu, workspace_name, sysroot_prefix))],
                ),
            ],
        ),
        feature(
            name = "default_link_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = _all_link_actions,
                    flag_groups = [flag_group(flags = _get_link_flags(cpu, workspace_name, sysroot_prefix))],
                ),
            ],
        ),
    ]

    # Windows hosts require .exe extensions on LLVM tool binaries
    ext = ".exe" if host_os == "windows" else ""
    
    tool_paths = [
        tool_path(name = "ar", path = prebuilt_dir + "/bin/llvm-ar" + ext),
        tool_path(name = "cpp", path = prebuilt_dir + "/bin/clang" + ext),
        tool_path(name = "gcc", path = prebuilt_dir + "/bin/clang++" + ext),
        tool_path(name = "gcov", path = "/bin/false"),
        tool_path(name = "ld", path = prebuilt_dir + "/bin/ld.lld" + ext),
        tool_path(name = "nm", path = prebuilt_dir + "/bin/llvm-nm" + ext),
        tool_path(name = "objcopy", path = prebuilt_dir + "/bin/llvm-objcopy" + ext),
        tool_path(name = "objdump", path = prebuilt_dir + "/bin/llvm-objdump" + ext),
        tool_path(name = "strip", path = prebuilt_dir + "/bin/llvm-strip" + ext),
    ]

    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        features = features,
        action_configs = [],
        cxx_builtin_include_directories = [
            prebuilt_dir + "/lib/clang/18/include",
            prebuilt_dir + "/lib/clang/19/include",
            prebuilt_dir + "/lib/clang/20/include",
            sysroot_prefix + "/usr/include",
            sysroot_prefix + "/usr/include/c++/v1",
        ],
        toolchain_identifier = "ndk-%s-toolchain" % cpu,
        host_system_name = "x86_64-unknown-linux-gnu" if host_os == "linux" else ("x86_64-apple-darwin" if host_os == "darwin" else "x86_64-pc-windows-msvc"),
        target_system_name = "arm-linux-android" if cpu == _ARMEABI_V7A else "aarch64-linux-android",
        target_cpu = cpu,
        target_libc = "android",
        compiler = "clang",
        cc_target_os = "android",
        tool_paths = tool_paths,
        builtin_sysroot = sysroot_prefix,
    )

ndk_cc_toolchain_config = rule(
    implementation = _ndk_cc_toolchain_config_impl,
    attrs = {
        "cpu": attr.string(mandatory = True, values = [_ARMEABI_V7A, _ARM64_V8A]),
        "host_os": attr.string(mandatory = True, values = ["linux", "darwin", "windows"]),
        "prebuilt_dir": attr.string(mandatory = True),
    },
    provides = [CcToolchainConfigInfo],
)
