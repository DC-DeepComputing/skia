:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

@echo off
for /d %%i in (external\*clang_windows_amd64) do (
    %%i\bin\llvm-ar.exe %*
    exit /b
)
for /d %%i in (..\..\..\..\..\..\..\external\*clang_windows_amd64) do (
    %%i\bin\llvm-ar.exe %*
    exit /b
)
