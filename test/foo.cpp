#include <iostream>
#include "test/foo.h"

Foo::Foo(std::string msg) {
  std::cout << "Foo constructed, msg: '" << msg << "'" << std::endl;
}

Foo foo2("Hello from foo.cpp!");
