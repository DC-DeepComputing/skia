:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

@echo off
:: Windows cmd.exe does not expand wildcards in executable paths. We must use a 'for /d'
:: directory-globbing loop to expand the dynamically-suffixed Bzlmod repository folder name
:: (e.g. +_repo_rules3+clang_windows_amd64) and bind the expanded path to %%i.
:: - %~dp0: Expands to the drive (d) and path (p) of the folder containing this batch script.
:: - %%~i: Expands %%i while stripping any surrounding quotes (important if paths contain spaces).
for /d %%i in ("%~dp0..\..\external\*clang_windows_amd64") do (
    :: We pass the '/lib' option to lld-link.exe to invoke it in MSVC-compatible static
    :: library manager mode much like llvm-ar.exe would work
    "%%~i\bin\lld-link.exe" /lib %*
    exit /b
)