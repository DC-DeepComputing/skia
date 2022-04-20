#include "base64.h"
#include "include/utils/SkBase64.h"
#include <string>

std::string base64_decode(std::string const& base64, bool) {
    size_t dataLen;
    if (SkBase64::Decode(base64.data(), base64.size(), nullptr, &dataLen) != SkBase64::kNoError) {
        return "";
    }

    std::string ret;
    ret.resize(dataLen);
    void* rawData = ret.data();
    if (SkBase64::Decode(base64.data(), base64.size(), rawData, &dataLen) != SkBase64::kNoError) {
        return "";
    }

    return ret;
}
