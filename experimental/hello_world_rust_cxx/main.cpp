#include "experimental/hello_world_rust_cxx/gen/hype-bridge.rs.h"

#include <stdio.h>
#include <string>


int main(int argc, char** argv) {
    printf("Hello C++\n");

    std::string words = "it works";

    HypeOutput result = hypeify(words, 3);

    printf("%s\n", result.output.c_str());
    printf("new len: %d\n", result.new_len);

    return 0;
}