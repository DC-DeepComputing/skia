### Compilation failed:

error: 3: cannot take the address of this expression
    x = &5;
        ^^
error: 5: cannot take the address of this expression
    x = &y.x;
        ^^^^
error: 6: cannot take the address of this expression
    x = &y[0];
        ^^^^^
3 errors
