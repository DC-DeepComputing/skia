:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

@echo off
toolchain\windows_trampolines\windows_trampoline.exe link_dynamic_library %*
if %errorlevel% neq 0 exit /b %errorlevel%
