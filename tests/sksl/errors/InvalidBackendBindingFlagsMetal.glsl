### Compilation failed:

<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
error: 13: layout qualifier 'sampler' is not permitted here
layout(metal, sampler=0) texture2D texture3;                        // invalid
^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(metal, binding=0, texture=0, sampler=0) sampler2D sampler3;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: layout qualifier 'texture' is not permitted here
layout(metal, texture=0, sampler=0) ubo2 { float c; };              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
=======
error: 14: layout qualifier 'sampler' is not permitted here
layout(metal, sampler=0) readonly texture2D rtexture3;              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
error: 15: layout qualifier 'sampler' is not permitted here
<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
layout(metal, texture=0, sampler=0) ubo2 { float c; };              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: layout qualifier 'set' is not permitted here
layout(metal, set=0, binding=0) ubo3 { float d; };                  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
5 errors
=======
layout(metal, sampler=0) writeonly texture2D wtexture3;             // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(metal, binding=0, texture=0, sampler=0) sampler2D sampler3;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: layout qualifier 'texture' is not permitted here
layout(metal, texture=0, sampler=0) ubo2 { float c; };              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: layout qualifier 'sampler' is not permitted here
layout(metal, texture=0, sampler=0) ubo2 { float c; };              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: layout qualifier 'set' is not permitted here
layout(metal, set=0, binding=0) ubo3 { float d; };                  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
6 errors
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
