### Compilation failed:

error: 7: atomics are only permitted in workgroup variables and storage blocks
atomic_uint globalAtomic;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: atomics are only permitted in workgroup variables and storage blocks
    atomic_uint localAtomic;  // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^
2 errors
