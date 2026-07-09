:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

@echo off
:: This batch script serves as a compiler trampoline to invoke the Clang
:: compiler located inside the dynamically-named external Bazel cache directory.
::
:: Since Bazel always executes build actions with the current working directory 
:: set to the execution root, we can locate the external toolchain repository 
:: directly under the relative "external\" folder. This avoids relative path 
:: parent traversals (like '..\..') which can trigger Windows cmd.exe wildcard bugs.
for /d %%i in ("external\*clang_windows_amd64") do (
    "%%~i\bin\clang.exe" %*
    exit /b
)
