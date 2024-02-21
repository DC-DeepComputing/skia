#include <iostream>
#include "test/foo.h"

Foo foo1("Hello from main.cpp!");

int main() {
  std::cout << "Hello, world!" << std::endl;
  return 0;
}
