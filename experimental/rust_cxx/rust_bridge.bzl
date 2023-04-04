# https://cxx.rs/build/bazel.html

load("@bazel_skylib//rules:run_binary.bzl", "run_binary")
load("@rules_cc//cc:defs.bzl", "cc_library")

def rust_cxx_bridge(name, src, deps = []):
    out_h = "gen/%s.h" % src
    out_cc = "gen/%s.cc" % src
    run_binary(
        name = "%s/generated" % name,
        srcs = [src],
        outs = [out_h, out_cc],
        args = [
            "$(location %s)" % src,
            "-o",
            "$(location %s)" % out_h,
            "-o",
            "$(location %s)" % out_cc,
        ],
        tool = "@cxxbridge_cmd//:cxxbridge",
    )

    cc_library(
        name = name,
        srcs = [out_cc],
        hdrs = [out_h],
        deps = deps,
    )

#
