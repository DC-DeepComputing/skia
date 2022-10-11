### Compilation failed:

error: unsupported texture type: 'texture2DRect'
error: unsupported sampler type: 'sampler2DRect'
error: 5: unsupported texture dimensions: 'sampler2DRect'
    c *= sample(t, coords);
         ^^^^^^^^^^^^^^^^^
error: 6: unsupported texture dimensions: 'sampler2DRect'
    c *= sampleGrad(t, coords, dFdx(coords), dFdy(coords));
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
