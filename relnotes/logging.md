Logging macros are now supported at a core Skia level. This update includes moving LoggingPriority
out of graphite and into core. SKGPU_GRAPHITE_LOWEST_ACTIVE_LOG_PRIORITY is still supported for
backwards compatibility, but new users are encouraged to use SKIA_LOWEST_ACTIVE_LOG_PRIORITY in
their config file.