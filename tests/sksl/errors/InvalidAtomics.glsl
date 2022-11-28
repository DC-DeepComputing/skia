### Compilation failed:

error: 4: atomics are only permitted in workgroup variables and writable storage blocks
atomic_uint globalAtomic;          // invalid
^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: atomics are only permitted in workgroup variables and writable storage blocks
atomic_uint globalAtomicArray[2];  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: struct with atomic member only permitted in workgroup variables and writable storage blocks
S globalStructWithAtomicMember;              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: struct with atomic member only permitted in workgroup variables and writable storage blocks
S globalStructWithAtomicMemberArray[2];      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: struct with atomic member only permitted in workgroup variables and writable storage blocks
NestedS globalStructWithNestedAtomicMember;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 32: variables of type 'atomic_uint' may not be uniform
    atomic_uint uboAtomic;                    // invalid
    ^^^^^^^^^^^^^^^^^^^^^^
error: 31: caused by:
layout(metal, binding = 1) uniform ubo {
                                   ^^^
error: 31: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 1) uniform ubo {
                                   ^^^
error: 39: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 2) readonly buffer roSsbo {
                                           ^^^^^^
error: 48: atomics are only permitted in workgroup variables and writable storage blocks
    atomic_uint localAtomic;                // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 49: atomics are only permitted in workgroup variables and writable storage blocks
    atomic_uint localAtomicArray[2];        // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 50: struct with atomic member only permitted in workgroup variables and writable storage blocks
    S localStructWithAtomicMember;          // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 51: struct with atomic member only permitted in workgroup variables and writable storage blocks
    S localStructWithAtomicMemberArray[2];  // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
13 errors
