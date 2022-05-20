### Compilation failed:

error: 6: variables of type 'bool' may not be uniform
uniform bool  b;
^^^^^^^^^^^^^^^
error: 7: variables of type 'bool2' may not be uniform
uniform bool2 b2;
^^^^^^^^^^^^^^^^
error: 8: variables of type 'bool3' may not be uniform
uniform bool3 b3;
^^^^^^^^^^^^^^^^
error: 9: variables of type 'bool4' may not be uniform
uniform bool4 b4;
^^^^^^^^^^^^^^^^
error: 14: type 'InvalidStruct1' contains a member that is disallowed in a uniform
    bool b; // invalid
    ^^^^^^
error: 21: variables of type 'InvalidStruct1' may not be uniform
uniform InvalidStruct1 st1;
^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: type 'InvalidStruct1' contains a member that is disallowed in a uniform
    bool b; // invalid
    ^^^^^^
error: 18: type 'InvalidStruct2' contains a member that is disallowed in a uniform
    InvalidStruct1 s;
    ^^^^^^^^^^^^^^^^
error: 22: variables of type 'InvalidStruct2' may not be uniform
uniform InvalidStruct2 st2;
^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 26: type 'invalidBlock' contains a member that is disallowed in a uniform
    bool b; // invalid
    ^^^^^^^
error: 24: variables of type 'invalidBlock' may not be uniform
uniform invalidBlock {
        ^^^^^^^^^^^^
11 errors
