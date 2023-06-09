### Compilation failed:

error: 8: unknown identifier 'width'
    float x = (float(sk_GlobalInvocationID.x * 2 - width(dest)) / float(width(dest)));
                                                   ^^^^^
error: 8: unknown identifier 'width'
    float x = (float(sk_GlobalInvocationID.x * 2 - width(dest)) / float(width(dest)));
                                                                        ^^^^^
error: 9: unknown identifier 'height'
    float y = (float(sk_GlobalInvocationID.y * 2 - height(dest)) / float(height(dest)));
                                                   ^^^^^^
error: 9: unknown identifier 'height'
    float y = (float(sk_GlobalInvocationID.y * 2 - height(dest)) / float(height(dest)));
                                                                         ^^^^^^
error: 11: unknown identifier 'x'
    float3 ray_target = float3(x * max_x, y * max_y, 0.0);
                               ^
error: 11: unknown identifier 'y'
    float3 ray_target = float3(x * max_x, y * max_y, 0.0);
                                          ^
error: 16: unknown identifier 'ray_target'
    float3 t_minus_c = ray_target - sphere_center;
                       ^^^^^^^^^^
error: 17: unknown identifier 't_minus_c'
    float b = dot(ray_origin, t_minus_c);
                              ^^^^^^^^^
error: 18: unknown identifier 't_minus_c'
    float c = dot(t_minus_c, t_minus_c) - sphere_radius * sphere_radius;
                  ^^^^^^^^^
error: 18: unknown identifier 't_minus_c'
    float c = dot(t_minus_c, t_minus_c) - sphere_radius * sphere_radius;
                             ^^^^^^^^^
error: 19: unknown identifier 'b'
    float bsqmc = b * b - c;
                  ^
error: 19: unknown identifier 'b'
    float bsqmc = b * b - c;
                      ^
error: 19: unknown identifier 'c'
    float bsqmc = b * b - c;
                          ^
error: 21: unknown identifier 'bsqmc'
    if (bsqmc >= 0.0) {
        ^^^^^
error: 25: unknown identifier 'write'
    write(dest, sk_GlobalInvocationID.xy, pixel);
    ^^^^^
15 errors
