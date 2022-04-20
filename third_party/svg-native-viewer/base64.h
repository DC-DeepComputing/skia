#ifndef SK_BASE64_H_FOR_SVGNATIVE
#define SK_BASE64_H_FOR_SVGNATIVE

#include <string>

// This is a repacement for SVGNative's https://github.com/ReneNyffenegger/cpp-base64 dependency.
std::string base64_decode(std::string const& base64, bool = false);

#endif
