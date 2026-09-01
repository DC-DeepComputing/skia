### Compilation failed:

error: 12: layout qualifier 'rgba32f' is not permitted here
layout(vulkan, rgba32f, input_attachment_index=0,
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 14: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0) readonly texture2D rtexture2;            // invalid (no set/binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: symbol 'rtexture2' was already defined
layout(vulkan, texture=0) readonly texture2D rtexture2;            // invalid (no set/binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: layout qualifier 'texture' is not permitted here
layout(vulkan, rgba32f, texture=0) writeonly texture2D wtexture2;  // invalid (no set/binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: storage textures must declare a pixel format
layout(vulkan, set=0, binding=0) writeonly texture2D wtexture3;    // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: layout qualifier 'texture' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;           // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: layout qualifier 'sampler' is not permitted here
layout(vulkan, texture=0, sampler=0) sampler2D sampler2;           // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
7 errors
