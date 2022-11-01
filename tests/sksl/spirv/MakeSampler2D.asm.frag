### Compilation failed:

error: 9: variables of type 'sampler2D' must be global
    sampler2D unified = makeSampler2D(t, s);
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: opaque type 'sampler2D' cannot use initializer expressions
    sampler2D unified = makeSampler2D(t, s);
                        ^^^^^^^^^^^^^^^^^^^
error: 10: call to 'get_texel' expected 2 arguments, but found 1
    return get_texel(xy);
           ^^^^^^^^^^^^^
3 errors
