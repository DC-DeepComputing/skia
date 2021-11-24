#ifndef Alpha_DEFINED
#define Alpha_DEFINED

#include <string>

std::string getAlphaSrc();

inline std::string getAlphaHdr() {
#ifdef SK_USE_ALPHA
#if SK_USE_ALPHA == 1
    return "alpha in header was 1";
#else
    return "alpha in header something else";
#endif
#else
    return "alpha was not actually defined in header";
#endif
}

#endif