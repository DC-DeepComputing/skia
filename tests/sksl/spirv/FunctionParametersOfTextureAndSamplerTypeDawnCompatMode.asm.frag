### Compilation failed:

error: 1: 'texture2D' requires a 'readonly' or 'writeonly' access qualifier
layout(set = 0, binding = 1) texture2D aTexture;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: 'texture2D' requires a 'readonly' or 'writeonly' access qualifier
noinline half4 helpers_helper(sampler2D s, texture2D t) {
                                           ^^^^^^^^^
error: 10: 'texture2D' requires a 'readonly' or 'writeonly' access qualifier
noinline half4 helper(texture2D t, sampler2D s) {
                      ^^^^^^^^^
3 errors
