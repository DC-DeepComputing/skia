"""
THIS IS THE EXTERNAL-ONLY VERSION OF THIS FILE. G3 HAS ITS OWN.

This file contains general helper macros that make our BUILD.bazel files easier to read.
"""

load("@bazel_gazelle//:def.bzl", _gazelle = "gazelle")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", _wasm_cc_binary = "wasm_cc_binary")
load("@io_bazel_rules_go//go:def.bzl", _go_binary = "go_binary", _go_library = "go_library")
load("@py_deps//:requirements.bzl", _requirement = "requirement")
load("@rules_python//python:defs.bzl", _py_binary = "py_binary")

gazelle = _gazelle
go_binary = _go_binary
go_library = _go_library
py_binary = _py_binary
requirement = _requirement
wasm_cc_binary = _wasm_cc_binary
