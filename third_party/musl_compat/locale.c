#include <stdlib.h>

long long strtoll_l(const char *nptr, char **endptr, int base,
                    int locale_t) {
  return strtoll(nptr, endptr, base);
}

long long strtoull_l(const char *nptr, char **endptr, int base,
                     int locale_t) {
  return strtoull(nptr, endptr, base);
}