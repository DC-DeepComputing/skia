### Compilation failed:

error: 1: only one backend qualifier can be used
layout(metal, vulkan, webgpu, direct3d, binding = 0) uniform ubo { float f; }; // multiple backends
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'texture' is not permitted here
<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
layout(texture=0, sampler=0) sampler2D s;                          // invalid (requires backend)
=======
layout(texture=0, sampler=0) sampler2D s;                                      // missing backend
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'sampler' is not permitted here
<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
layout(texture=0, sampler=0) sampler2D s;                          // invalid (requires backend)
=======
layout(texture=0, sampler=0) sampler2D s;                                      // missing backend
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
3 errors
