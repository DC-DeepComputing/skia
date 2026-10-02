/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emscripten/wasmfs.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "include/private/SkDebug.h"

// The emscripten directory backed by Chrome's OPFS.
#ifndef SK_WASMFS_OPFS_MOUNT_ROOT
#define SK_WASMFS_OPFS_MOUNT_ROOT "/opfs"
#endif

extern "C" int __real___main_argc_argv(int argc, char** argv);

// WasmFS for some reason has a teardown assertion that all fds have been
// closed. Skia tools may not always do that, so make a best effort here.
static void close_open_files() {
    for (int fd = 3; fd < 1024; ++fd) {
        if (close(fd) == 0){
            SkDEBUGF("wasmfs: closed leaked fd %d\n", fd);
        }
    }
}

static int mount_opfs(const char* path) {
    backend_t opfs = wasmfs_create_opfs_backend();
    if (!opfs) {
        SkDEBUGF("wasmfs: failed to create OPFS backend\n");
        return 1;
    }

    if (int rc = wasmfs_create_directory(path, 0777, opfs)) {
        SkDEBUGF("wasmfs: create dir %s failed: %d\n", path, rc);
        return 1;
    }

    atexit(close_open_files);
    SkDEBUGF("wasmfs: Chrome OPFS mount at %s\n", path);
    return 0;
}

// Emscripten looks for the literal symbol `__main_argc_argv` when wiring up
// main's args. Since we've replaced the normal symbol with our two here
// (__wrap and __real), emscripten won't find it anymore, and will omit argc and
// argv. Renaming the export allows emscripten to find our wrapper main as if it
// were the original unwrapped main.
extern "C" __attribute__((export_name("__main_argc_argv")))
int __wrap___main_argc_argv(int argc, char** argv) {
    if (mount_opfs(SK_WASMFS_OPFS_MOUNT_ROOT)) {
        return 1;
    }

    SkDEBUGF("wasmfs: hooks installed; argc=%d\n", argc);
    return __real___main_argc_argv(argc, argv);
}
