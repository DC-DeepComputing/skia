### Compilation failed:

error: 11: symbol 'rtexture2' was already defined
layout(webgpu, set=0, binding=0) readonly texture2D rtexture2;             // valid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: layout qualifier 'sampler' is not permitted here
layout(webgpu, set=0, sampler=0) readonly texture2D rtexture3;             // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: symbol 'rtexture3' was already defined
layout(webgpu, set=0, sampler=0) readonly texture2D rtexture3;             // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: layout qualifier 'sampler' is not permitted here
layout(webgpu, rgba32f, set=0, sampler=0) writeonly texture2D wtexture3;   // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: storage textures must declare a pixel format
layout(webgpu, set=0, texture=0) writeonly texture2D wtexture4;            // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(webgpu, set=0, binding=0, texture=0, sampler=0) sampler2D sampler3; // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: layout qualifier 'texture' is not permitted here
layout(webgpu, set=0, texture=0, sampler=0) uniform ubo2 { float c; };     // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: layout qualifier 'sampler' is not permitted here
layout(webgpu, set=0, texture=0, sampler=0) uniform ubo2 { float c; };     // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
8 errors
