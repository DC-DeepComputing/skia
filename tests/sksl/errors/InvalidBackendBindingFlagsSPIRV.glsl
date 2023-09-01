### Compilation failed:

error: 9: layout qualifier 'texture' is not permitted here
layout(vulkan, rgba32f, texture=0) readonly texture2D rtexture2;   // invalid (no set/binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: layout qualifier 'texture' is not permitted here
layout(vulkan, rgba32f, texture=0) writeonly texture2D wtexture2;  // invalid (no set/binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: storage textures must declare a pixel format
layout(vulkan, set=0, binding=0) readonly texture2D rtexture3;     // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 12: storage textures must declare a pixel format
layout(vulkan, set=0, binding=0) writeonly texture2D wtexture3;    // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;           // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 13: layout qualifier 'sampler' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;           // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
6 errors
