#include "experimental/bazel_test/alpha/Alpha.h"

std::string getAlphaSrc() {
#ifdef SK_USE_ALPHA
#if SK_USE_ALPHA == 1
    return "alpha in source was 1";
#else
    return "alpha in source something else";
#endif
#else
    return "alpha was not actually defined in source";
#endif
}