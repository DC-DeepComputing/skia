`ContextOptions.fEnableCapture` can now be used to capture all Skia draws in Recorders made from
the context. After setting the option, calling `beginCapture` and `endCapture` on the Context will
control when the Recorders are actively capturing.