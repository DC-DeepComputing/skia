### Compilation failed:

error: 1: only one backend qualifier can be used
layout(metal, spirv, wgsl, gl, binding = 0) uniform ubo1 { float f1; };  // multiple backends
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'texture' is not permitted here
layout(texture=0, sampler=0) uniform sampler2D s1;  // invalid (requires backend)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: layout qualifier 'sampler' is not permitted here
layout(texture=0, sampler=0) uniform sampler2D s1;  // invalid (requires backend)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: layout qualifier 'sampler' is not permitted here
layout(metal, sampler=0) uniform texture2D t3;                        // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(metal, binding=0, texture=0, sampler=0) uniform sampler2D s4;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: layout qualifier 'texture' is not permitted here
layout(metal, texture=0, sampler=0) uniform ubo3 { float f4; };       // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: layout qualifier 'sampler' is not permitted here
layout(metal, texture=0, sampler=0) uniform ubo3 { float f4; };       // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: layout qualifier 'set' is not permitted here
layout(metal, set=0, binding=0) uniform ubo4 { float f5; };           // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 34: layout qualifier 'texture' is not permitted here
layout(spirv, texture=0) uniform texture2D t5;             // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 35: layout qualifier 'texture' is not permitted here
layout(spirv, texture=0, sampler=0) uniform sampler2D s6;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 35: layout qualifier 'sampler' is not permitted here
layout(spirv, texture=0, sampler=0) uniform sampler2D s6;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 52: layout qualifier 'sampler' is not permitted here
layout(wgsl, set=0, sampler=0) uniform texture2D t8;                        // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 53: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(wgsl, set=0, binding=0, texture=0, sampler=0) uniform sampler2D s9;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 54: layout qualifier 'texture' is not permitted here
layout(wgsl, set=0, texture=0, sampler=0) uniform ubo7 { float f10; };      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 54: layout qualifier 'sampler' is not permitted here
layout(wgsl, set=0, texture=0, sampler=0) uniform ubo7 { float f10; };      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 66: layout qualifier 'texture' is not permitted here
layout(gl, texture=0) uniform texture2D t10;              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 67: layout qualifier 'texture' is not permitted here
layout(gl, texture=0, sampler=0) uniform sampler2D s11;   // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 67: layout qualifier 'sampler' is not permitted here
layout(gl, texture=0, sampler=0) uniform sampler2D s11;   // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 68: layout qualifier 'set' is not permitted here
layout(gl, set=0, binding=0) uniform ubo9 { float f13; }; // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
19 errors
