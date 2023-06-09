### Compilation failed:

error: 9: unknown identifier 'width'
    if (sk_GlobalInvocationID.x < width(tex) && sk_GlobalInvocationID.y < height(tex)) {
                                  ^^^^^
error: 9: unknown identifier 'height'
    if (sk_GlobalInvocationID.x < width(tex) && sk_GlobalInvocationID.y < height(tex)) {
                                                                          ^^^^^^
error: 10: unknown identifier 'write'
        write(tex, sk_GlobalInvocationID.xy, desaturate(read(tex, sk_GlobalInvocationID.xy)));
        ^^^^^
error: 10: unknown identifier 'read'
        write(tex, sk_GlobalInvocationID.xy, desaturate(read(tex, sk_GlobalInvocationID.xy)));
                                                        ^^^^
4 errors
