### Compilation failed:

error: 3: atomics are only permitted in workgroup variables and writable storage blocks
    switch (x) { case 0: atomic_uint[7] a, b = a; }
                         ^^^^^^^^^^^^^^^^
error: 3: atomics are only permitted in workgroup variables and writable storage blocks
    switch (x) { case 0: atomic_uint[7] a, b = a; }
                                           ^^^^^
error: 3: opaque type 'atomic_uint[7]' cannot use initializer expressions
    switch (x) { case 0: atomic_uint[7] a, b = a; }
                                               ^
3 errors
