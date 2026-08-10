Added the `skia_enable_threadlocal_strikecache` GN build argument
(which defines `SK_ENABLE_THREADLOCAL_STRIKECACHE`) to enable
per-thread `SkStrikeCache` instances for improved multi-threaded text rendering performance.

