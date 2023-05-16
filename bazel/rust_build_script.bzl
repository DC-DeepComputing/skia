"""
This file specifies a macro that is a thin wrapper around the `cargo_build_script` rule.
The main thing this does is define an environment variable called `WORKSPACE_DIR` which points at
the root of the Rust project.
"""

load("@rules_rust//cargo:defs.bzl", "cargo_build_script")

def rust_build_script(name, srcs, data = [], deps = [], crate_features = [], env = []):
    cargo_build_script(
        name = name,
        srcs = srcs,
        data = data,
        deps = deps,
        crate_features = crate_features,
        build_script_env = env,
    )
