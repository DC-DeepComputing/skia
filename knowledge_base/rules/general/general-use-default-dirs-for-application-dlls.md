---
name: skia/general-use-default-dirs-for-application-dlls
description: When dynamically loading application-bundled or redistributable DLLs via LoadLibraryEx on Windows, use LOAD_LIBRARY_SEARCH_DEFAULT_DIRS instead of LOAD_LIBRARY_SEARCH_SYSTEM32 or LOAD_LIBRARY_REQUIRE_SIGNED_TARGET
rubric: correctness
severity: should_fix
---

**Rule.** When loading dynamic libraries via LoadLibraryExW or LoadLibraryExA on
Windows that may be bundled with the application or distributed via runtime SDKs
(such as DWriteCore.dll), specify LOAD_LIBRARY_SEARCH_DEFAULT_DIRS. Do not
restrict the search path to LOAD_LIBRARY_SEARCH_SYSTEM32, and do not enforce
LOAD_LIBRARY_REQUIRE_SIGNED_TARGET.

**Why.** Restricting the search to LOAD_LIBRARY_SEARCH_SYSTEM32 causes DLL
loading to fail for libraries packaged alongside the application or shipped via
modern SDKs (like Windows App SDK) rather than installed in
%SystemRoot%\\System32. Specifying LOAD_LIBRARY_SEARCH_DEFAULT_DIRS searches the
application directory, System32, and user-added directories while still
preventing DLL search-order hijacking by omitting the working directory and
PATH. Enforcing LOAD_LIBRARY_REQUIRE_SIGNED_TARGET unnecessarily breaks
execution in unsigned test environments and custom developer builds.

## Examples

Bad:

```cpp
dWriteCreateFactoryProc = reinterpret_cast<DWriteCreateFactoryProc>(GetProcAddress(
        LoadLibraryExW(L"DWriteCore.dll",
                       nullptr,
                       LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_REQUIRE_SIGNED_TARGET),
        "DWriteCoreCreateFactory"));
```

Good:

```cpp
dWriteCreateFactoryProc = reinterpret_cast<DWriteCreateFactoryProc>(GetProcAddress(
        LoadLibraryExW(L"DWriteCore.dll", nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS),
        "DWriteCoreCreateFactory"));
```
