### Compilation failed:

<<<<<<< PATCH SET (0e6ef0 Remove SkTime from the public API)
error: 1: variables of type 'texture2D' may not be uniform
uniform texture2D s
^^^^^^^^^^^^^^^^^^^
1 error
=======
error: 1: 'texture2D' requires a 'readonly' or 'writeonly' access qualifier
uniform texture2D s
^^^^^^^
error: 1: variables of type 'texture2D' may not be uniform
uniform texture2D s
^^^^^^^^^^^^^^^^^^^
2 errors
>>>>>>> BASE      (f3f6c7 Revert "Roll Dawn from beaf20f90f1b to b9afa50913de (668 rev)
