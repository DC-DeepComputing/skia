### Compilation failed:

error: 1: unsized arrays are only permitted in storage blocks
in int[] a;
^^^^^^^^^^
error: 1: user-defined stage in/out variables not allowed in compute shaders
in int[] a;
^^^^^^^^^^
error: 1: 'in' variables may not have unsized array type
in int[] a;
^^^^^^^^^^
error: 3: unsized array must be the last member of a storage block
uniform blockOne {
        ^^^^^^^^
error: 7: unsized array must be the last member of a storage block
buffer blockTwo {
       ^^^^^^^^
error: 12: unsized array must be the last member of a storage block
buffer blockThree {
       ^^^^^^^^^^
error: 18: unsized arrays are only permitted in storage blocks
    int[] g;
    ^^^^^^^
error: 18: function variable cannot be an unsized array
    int[] g;
    ^^^^^^^^
8 errors
