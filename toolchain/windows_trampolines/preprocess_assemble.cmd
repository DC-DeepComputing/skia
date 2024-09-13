:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

@echo off
bazel-out\x64_windows-opt-exec-ST-29e576965165\bin\toolchain\windows_trampoline_\windows_trampoline.exe preprocess_assemble %*
if %errorlevel% neq 0 exit /b %errorlevel%
