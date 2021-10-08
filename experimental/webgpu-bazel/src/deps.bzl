load("//toolchain:cc_toolchain_config.bzl", "emsdk_configure")

def load_emsdk_files():
    # Make all files under submodules/emsdk/* visible to the toolchain. The files are
    # available as external/emsdk/emsdk/*
    emsdk_configure(name = "emsdk")
