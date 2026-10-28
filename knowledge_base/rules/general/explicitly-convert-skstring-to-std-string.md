---
name: skia/explicitly-convert-skstring-to-std-string
description: When returning or assigning an SkString or SkStringPrintf result in contexts expecting std::string or std::string_view, explicitly call .c_str() or construct std::string
rubric: correctness
severity: blocking
---

**Rule.** When returning, assigning, or passing an SkString (such as from
SkStringPrintf) in a context expecting std::string or std::string_view,
explicitly call .c_str() or construct std::string. Do not rely on implicit
conversion between SkString and standard string types.

**Why.** SkString does not implicitly convert to std::string or
std::string_view. Passing or returning SkString directly where a standard string
type is expected causes compilation failures that can slip past CQ on targets
only built in specific configurations or build systems (such as Bazel
testrunners).

## Examples

Bad:

```cpp
std::string get_tf_name(const skcms_TransferFunction& tf) {
    switch (classify(tf)) {
        case skcms_TFType_PQ:
            return SkStringPrintf("PQ %.3g", tf.a);
        default:
            return "Unknown";
    }
}
```

Good:

```cpp
std::string get_tf_name(const skcms_TransferFunction& tf) {
    switch (classify(tf)) {
        case skcms_TFType_PQ:
            return SkStringPrintf("PQ %.3g", tf.a).c_str();
        default:
            return "Unknown";
    }
}
```
