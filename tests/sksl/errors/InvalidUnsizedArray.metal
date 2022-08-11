### Compilation failed:

error: 1: unsized arrays are not permitted here
in int[] a;
^^^^^^^^^^
error: 1: pipeline inputs not permitted in compute shaders
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
error: 18: unsized arrays are not permitted here
    int[] g;
    ^^^^^^^
error: 18: unsized arrays are not permitted here
    int[] g;
    ^^^^^^^^
8 errors
