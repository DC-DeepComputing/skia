
The SkExecutor class is now templated so it can support different work submission models.

The prior std::function-based model is preserved as SkStdFunctionExecutor and SkExecutor.

A new callback-style model is now supported with SkCallbackExecutor.

Graphite now takes a SkCallbackExecutor in its ContextOptions while Ganesh continues to take a SkStdFunctionExecutor.
