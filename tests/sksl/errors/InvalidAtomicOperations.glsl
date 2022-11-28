### Compilation failed:

error: 18: type mismatch: '=' cannot operate on 'atomic_uint', 'int'
    wgCounterA = 1;
    ^^^^^^^^^^^^^^
error: 19: cannot construct 'atomic_uint'
    wgCounterA = atomic_uint(1);
                 ^^^^^^^^^^^^^^
error: 21: '+' cannot operate on 'atomic_uint'
    +wgCounterA;
    ^^^^^^^^^^^
error: 22: '-' cannot operate on 'atomic_uint'
    -wgCounterA;
    ^^^^^^^^^^^
error: 23: '!' cannot operate on 'atomic_uint'
    !wgCounterA;
    ^^^^^^^^^^^
error: 25: '++' cannot operate on 'atomic_uint'
    wgCounterA++;
    ^^^^^^^^^^^^
error: 26: '--' cannot operate on 'atomic_uint'
    wgCounterA--;
    ^^^^^^^^^^^^
error: 27: '++' cannot operate on 'atomic_uint'
    ++wgCounterA;
    ^^^^^^^^^^^^
error: 28: '--' cannot operate on 'atomic_uint'
    --wgCounterA;
    ^^^^^^^^^^^^
error: 30: '-' cannot operate on 'atomic_uint'
    wgCounterA = -wgCounterA;
                 ^^^^^^^^^^^
error: 31: '+' cannot operate on 'atomic_uint'
    wgCounterA = +wgCounterA;
                 ^^^^^^^^^^^
error: 32: assignments to atomic type 'atomic_uint' are not permitted
    wgCounterA = wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 33: type mismatch: '+=' cannot operate on 'atomic_uint', 'atomic_uint'
    wgCounterA += wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 34: type mismatch: '+' cannot operate on 'atomic_uint', 'atomic_uint'
    wgCounterA = wgCounterA + wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 35: type mismatch: '-' cannot operate on 'atomic_uint', 'atomic_uint'
    wgCounterA = wgCounterA - wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 36: type mismatch: '*' cannot operate on 'atomic_uint', 'atomic_uint'
    wgCounterA = wgCounterA * wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 37: type mismatch: '/' cannot operate on 'atomic_uint', 'atomic_uint'
    wgCounterA = wgCounterA / wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 39: expected 'uint', but found 'atomic_uint'
    uint a = wgCounterA;
             ^^^^^^^^^^
error: 40: construction of struct type 'S' with atomic member is not allowed
    wgStructWithAtomicMember = S(1);
                               ^^^^
error: 41: cannot construct 'atomic_uint'
    wgStructWithAtomicMember = S(atomic_uint(1));
                                 ^^^^^^^^^^^^^^
error: 41: construction of struct type 'S' with atomic member is not allowed
    wgStructWithAtomicMember = S(atomic_uint(1));
                               ^^^^^^^^^^^^^^^^^
error: 42: construction of struct type 'S' with atomic member is not allowed
    wgStructWithAtomicMember = S(wgCounterA);
                               ^^^^^^^^^^^^^
error: 44: assignments to atomic type 'atomic_uint' are not permitted
    wgAtomicArray[0] = wgCounterA;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 45: assignments to atomic type 'atomic_uint' are not permitted
    wgAtomicArray[1] = wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 46: assignments to atomic type 'atomic_uint[2]' are not permitted
    wgAtomicArray = wgAtomicArray2;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 47: construction of array type 'atomic_uint[2]' with atomic member is not allowed
    wgAtomicArray = atomic_uint[2](wgCounterA, wgCounterB);
                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 50: construction of array type 'S[2]' with atomic member is not allowed
    wgStructWithAtomicMemberArray = S[2](wgStructWithAtomicMember,
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 52: construction of struct type 'NestedS' with atomic member is not allowed
    wgNestedStructWithAtomicMember = NestedS(wgStructWithAtomicMember);
                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
28 errors
