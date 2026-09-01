### Compilation failed:

error: 11: symbol 'rtexture2' was already defined
layout(metal, binding=0) readonly texture2D rtexture2;                     // valid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: layout qualifier 'sampler' is not permitted here
layout(metal, sampler=0) readonly texture2D rtexture3;                     // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: symbol 'rtexture3' was already defined
layout(metal, sampler=0) readonly texture2D rtexture3;                     // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: layout qualifier 'sampler' is not permitted here
layout(metal, rgba32f, sampler=0) writeonly texture2D wtexture3;           // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: layout qualifier 'sampler' is not permitted here
layout(metal, texture=0, sampler=0) readonly texture2D rtexture4;          // invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: layout qualifier 'sampler' is not permitted here
layout(metal, rgba32f, texture=0, sampler=0) writeonly texture2D wtexture4;// invalid (has sampler)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: storage textures must declare a pixel format
layout(metal, texture=0) writeonly texture2D wtexture5;                    // invalid (no pixformat)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: 'binding' modifier cannot coexist with 'texture'/'sampler'
layout(metal, binding=0, texture=0, sampler=0) sampler2D sampler3;         // invalid (has binding)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: layout qualifier 'texture' is not permitted here
layout(metal, texture=0, sampler=0) ubo2 { float c; };                     // invalid (has tex/samp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: layout qualifier 'sampler' is not permitted here
layout(metal, texture=0, sampler=0) ubo2 { float c; };                     // invalid (has tex/samp)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 23: layout qualifier 'set' is not permitted here
layout(metal, set=0, binding=0) ubo3 { float d; };                         // invalid (has set)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
11 errors
