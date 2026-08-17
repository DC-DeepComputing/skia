def _check_compiler_impl(ctx):
    # Try to find clang on the host system
    compiler = ctx.which("clang")

    supports_flag = False
    if compiler:
        # Devnull equivalent for cross-platform compatibility
        devnull = "NUL" if ctx.os.name.startswith("windows") else "/dev/null"

        # Execute the compiler check (just like the python script)
        res = ctx.execute([
            compiler,
            "-Werror",
            "--warning-suppression-mappings=" + devnull,
            "-c",
            "-x",
            "c",
            devnull,
            "-o",
            devnull,
        ])

        # If return code is 0, the compiler accepts the flag
        supports_flag = (res.return_code == 0)

    # Generate an empty BUILD file so Bazel treats this as a valid package
    ctx.file("BUILD.bazel", "")

    # Generate a .bzl file that exports the boolean result
    ctx.file("compiler_info.bzl", "SUPPORTS_UNSAFE_BUFFERS = %s\n" % supports_flag)

# Define the repository rule
check_clang_features = repository_rule(
    implementation = _check_compiler_impl,
    local = True,  # Tells Bazel to re-run this if the local system environment changes
)

# Define the module extension for Bzlmod
def _compiler_ext_impl(ctx):
    check_clang_features(name = "clang_features")

compiler_ext = module_extension(
    implementation = _compiler_ext_impl,
)
