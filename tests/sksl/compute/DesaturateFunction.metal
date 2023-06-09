### Compilation failed:

error: 5: unknown identifier 'read'
    half4 color = read(src, sk_GlobalInvocationID.xy);
                  ^^^^
error: 6: unknown identifier 'color'
    color.rgb = half3(dot(color.rgb, half3(0.22, 0.67, 0.11)));
    ^^^^^
error: 6: unknown identifier 'color'
    color.rgb = half3(dot(color.rgb, half3(0.22, 0.67, 0.11)));
                          ^^^^^
error: 7: unknown identifier 'write'
    write(dest, sk_GlobalInvocationID.xy, color);
    ^^^^^
error: 7: unknown identifier 'color'
    write(dest, sk_GlobalInvocationID.xy, color);
                                          ^^^^^
error: 11: unknown identifier 'width'
    if (sk_GlobalInvocationID.x < width(src) && sk_GlobalInvocationID.y < height(src)) {
                                  ^^^^^
error: 11: unknown identifier 'height'
    if (sk_GlobalInvocationID.x < width(src) && sk_GlobalInvocationID.y < height(src)) {
                                                                          ^^^^^^
7 errors
