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
layout(metal, binding = 1) uniform ubo1 {
                                   ^^^^
error: 31: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 1) uniform ubo1 {
                                   ^^^^
error: 35: variables of type 'atomic_uint[2]' may not be uniform
    atomic_uint uboAtomicArray[2];            // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 34: caused by:
layout(metal, binding = 2) uniform ubo2 {
                                   ^^^^
error: 34: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 2) uniform ubo2 {
                                   ^^^^
error: 8: variables of type 'atomic_uint' may not be uniform
    atomic_uint structMemberAtomic;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 37: caused by:
layout(metal, binding = 3) uniform ubo3 {
                                   ^^^^
error: 37: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 3) uniform ubo3 {
                                   ^^^^
error: 40: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 4) uniform ubo4 {
                                   ^^^^
error: 8: variables of type 'atomic_uint' may not be uniform
    atomic_uint structMemberAtomic;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 43: caused by:
layout(metal, binding = 5) uniform ubo5 {
                                   ^^^^
error: 43: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 5) uniform ubo5 {
                                   ^^^^
error: 47: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 6) readonly buffer roSsbo1 {
                                           ^^^^^^^
error: 50: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 7) readonly buffer roSsbo2 {
                                           ^^^^^^^
error: 53: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 8) readonly buffer roSsbo3 {
                                           ^^^^^^^
error: 56: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 9) readonly buffer roSsbo4 {
                                           ^^^^^^^
error: 59: struct with atomic member only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 10) readonly buffer roSsbo5 {
                                            ^^^^^^^
error: 64: atomics are only permitted in workgroup variables and writable storage blocks
    atomic_uint localAtomic;                // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 65: atomics are only permitted in workgroup variables and writable storage blocks
    atomic_uint localAtomicArray[2];        // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 66: struct with atomic member only permitted in workgroup variables and writable storage blocks
    S localStructWithAtomicMember;          // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 67: struct with atomic member only permitted in workgroup variables and writable storage blocks
    S localStructWithAtomicMemberArray[2];  // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
27 errors
