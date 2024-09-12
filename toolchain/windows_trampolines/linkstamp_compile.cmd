:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

@echo off
toolchain\windows_trampolines\windows_trampoline.exe linkstamp_compile %*
if %errorlevel% neq 0 exit /b %errorlevel%
