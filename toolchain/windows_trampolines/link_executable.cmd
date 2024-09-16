:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

@echo off
bazel-out\x64_windows-opt-exec-D7A71F67\bin\toolchain\windows_trampoline_\windows_trampoline.exe link_executable %*
if %errorlevel% neq 0 exit /b %errorlevel%
