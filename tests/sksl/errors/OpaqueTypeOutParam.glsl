### Compilation failed:

error: 4: 'out' is not permitted here
void func2(out sampler2D s)   { /*error*/ }
           ^^^
error: 5: 'out' is not permitted here
void func3(inout sampler2D s) { /*error*/ }
           ^^^^^
2 errors
