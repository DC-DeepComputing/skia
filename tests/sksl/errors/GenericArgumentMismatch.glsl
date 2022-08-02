### Compilation failed:

error: 1: no match for sin(int, int)
considered:
$genHType sin($genHType angle)
$genType sin($genType angle)

void sin_int_int() { float x = sin(1, 2); }
                               ^^^^^^^^^
error: 2: no match for sin(bool)
considered:
$genHType sin($genHType angle)
$genType sin($genType angle)

void sin_bool()    { float x = sin(true); }
                               ^^^^^^^^^
2 errors
