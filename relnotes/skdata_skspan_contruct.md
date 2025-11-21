
SkSpan-based overloads of the `SkData::MakeWithCopy()` and `SkData::MakeWithoutCopy()` methods are introduced,
allowing the compiler to deduce sizes on behalf of many callers, avoiding the possibility of a size mismatch
when using the versions that take discrete pointer / size arguments.
