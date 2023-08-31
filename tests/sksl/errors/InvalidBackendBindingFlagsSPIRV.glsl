### Compilation failed:

<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
error: 8: layout qualifier 'texture' is not permitted here
layout(spirv, texture=0) texture2D texture2;                 // invalid
^^^^^^^^^^^^^^^^^^^^^^^^
=======
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
error: 9: layout qualifier 'texture' is not permitted here
<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
layout(spirv, texture=0, sampler=0) sampler2D sampler2;      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: layout qualifier 'sampler' is not permitted here
layout(spirv, texture=0, sampler=0) sampler2D sampler2;      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
=======
layout(vulkan, texture=0) readonly texture2D rtexture2;   // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0) writeonly texture2D wtexture2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: layout qualifier 'sampler' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
