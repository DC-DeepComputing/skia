### Compilation failed:

error: 11: expected 'texture2D', but found 'readonlyTexture2D'
    needs_all_access(src);                          // BAD
                     ^^^
error: 12: expected 'texture2D', but found 'writeonlyTexture2D'
    needs_all_access(dest);                         // BAD
                     ^^^^
error: 15: unknown identifier 'read'
    read(dest, sk_GlobalInvocationID.xy);           // BAD
    ^^^^
error: 16: unknown identifier 'write'
    write(src, sk_GlobalInvocationID.xy, half4(1)); // BAD
    ^^^^^
error: 19: no match for overload(readonlyTexture2D, int)
    overload(src, 1);  // BAD: overload(readonly texture2D t, int) missing
    ^^^^^^^^^^^^^^^^
error: 20: no match for overload(writeonlyTexture2D)
    overload(dest);    // BAD: overload(writeonly texture2D t)      missing
    ^^^^^^^^^^^^^^
error: 26: unknown identifier 'width'
    width(t);                                     // OK
    ^^^^^
error: 27: unknown identifier 'read'
    read(t, sk_GlobalInvocationID.xy);            // OK
    ^^^^
error: 28: unknown identifier 'write'
    write(t, sk_GlobalInvocationID.xy, half4(1)); // OK
    ^^^^^
error: 32: expected 'texture2D', but found 'readonlyTexture2D'
    needs_all_access(t);                          // BAD
                     ^
error: 33: unknown identifier 'width'
    width(t);                                     // OK
    ^^^^^
error: 34: unknown identifier 'read'
    read(t, sk_GlobalInvocationID.xy);            // OK
    ^^^^
error: 35: unknown identifier 'write'
    write(t, sk_GlobalInvocationID.xy, half4(1)); // BAD
    ^^^^^
error: 39: expected 'texture2D', but found 'writeonlyTexture2D'
    needs_all_access(t);                          // BAD
                     ^
error: 40: unknown identifier 'width'
    width(t);                                     // OK
    ^^^^^
error: 41: unknown identifier 'read'
    read(t, sk_GlobalInvocationID.xy);            // BAD
    ^^^^
error: 42: unknown identifier 'write'
    write(t, sk_GlobalInvocationID.xy, half4(1)); // OK
    ^^^^^
17 errors
