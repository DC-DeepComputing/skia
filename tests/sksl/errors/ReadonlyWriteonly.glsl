### Compilation failed:

error: 11: argument 1 to 'void needs_all_access(texture2D t)' must be 'texture2D'
    needs_all_access(src);                      // BAD
                     ^^^
error: 12: argument 1 to 'void needs_all_access(texture2D t)' must be 'texture2D'
    needs_all_access(dest);                     // BAD
                     ^^^^
error: 15: argument 1 to 'half4 read(texture2D t, uint2 pos)' must be 'readonly texture2D'
    read(dest, sk_ThreadPosition.xy);           // BAD
         ^^^^
error: 16: argument 1 to 'void write(texture2D t, uint2 pos, half4 color)' must be 'writeonly texture2D'
    write(src, sk_ThreadPosition.xy, half4(1)); // BAD
          ^^^
error: 19: no match for overload(texture2D, int)
    overload(src, 1);  // BAD: overload(readonly texture2D t, int) missing
    ^^^^^^^^^^^^^^^^
error: 20: no match for overload(texture2D)
    overload(dest);    // BAD: overload(writeonly texture2D t)      missing
    ^^^^^^^^^^^^^^
error: 32: argument 1 to 'void needs_all_access(texture2D t)' must be 'texture2D'
    needs_all_access(t);                      // BAD
                     ^
error: 35: argument 1 to 'void write(texture2D t, uint2 pos, half4 color)' must be 'writeonly texture2D'
    write(t, sk_ThreadPosition.xy, half4(1)); // BAD
          ^
error: 39: argument 1 to 'void needs_all_access(texture2D t)' must be 'texture2D'
    needs_all_access(t);                      // BAD
                     ^
error: 41: argument 1 to 'half4 read(texture2D t, uint2 pos)' must be 'readonly texture2D'
    read(t, sk_ThreadPosition.xy);            // BAD
         ^
error: 46: argument 1 to 'void needs_all_access(texture2D t)' must be 'texture2D'
    needs_all_access(t);                      // BAD
                     ^
error: 48: argument 1 to 'half4 read(texture2D t, uint2 pos)' must be 'readonly texture2D'
    read(t, sk_ThreadPosition.xy);            // BAD
         ^
error: 49: argument 1 to 'void write(texture2D t, uint2 pos, half4 color)' must be 'writeonly texture2D'
    write(t, sk_ThreadPosition.xy, half4(1)); // BAD
          ^
13 errors
