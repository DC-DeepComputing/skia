### Compilation failed:

error: 10: unknown identifier 'width'
    if (sk_GlobalInvocationID.x < width(src) && sk_GlobalInvocationID.y < height(src)) {
                                  ^^^^^
error: 10: unknown identifier 'height'
    if (sk_GlobalInvocationID.x < width(src) && sk_GlobalInvocationID.y < height(src)) {
                                                                          ^^^^^^
error: 11: unknown identifier 'write'
        write(dest, sk_GlobalInvocationID.xy, desaturate(read(src, sk_GlobalInvocationID.xy)));
        ^^^^^
error: 11: unknown identifier 'read'
        write(dest, sk_GlobalInvocationID.xy, desaturate(read(src, sk_GlobalInvocationID.xy)));
                                                         ^^^^
4 errors
