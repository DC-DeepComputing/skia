### Compilation failed:

error: 1: user-defined stage in/out variables not allowed in compute shaders
layout(set=0, binding=0) in int x;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: user-defined stage in/out variables not allowed in compute shaders
layout(set=0, binding=1) out float4 y;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: user-defined stage in/out variables not allowed in compute shaders
layout(set=0, binding=2) in int[5] z;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: unsized arrays are only permitted in storage blocks
layout(set=0, binding=3) out int[] a;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: user-defined stage in/out variables not allowed in compute shaders
layout(set=0, binding=3) out int[] a;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 4: 'out' variables may not have unsized array type
layout(set=0, binding=3) out int[] a;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: unsized arrays are only permitted in storage blocks
layout(set=0, binding=4) in int[] b;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: user-defined stage in/out variables not allowed in compute shaders
layout(set=0, binding=4) in int[] b;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 5: 'in' variables may not have unsized array type
layout(set=0, binding=4) in int[] b;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: user-defined stage in/out variables not allowed in compute shaders
layout(set=0, binding=5) in blockOne {
                            ^^^^^^^^
error: 9: user-defined stage in/out variables not allowed in compute shaders
layout(set=0, binding=5) out blockTwo {
                             ^^^^^^^^
11 errors
