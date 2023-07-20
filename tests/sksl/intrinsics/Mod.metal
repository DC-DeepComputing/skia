### Compilation failed:

error: 5: unknown identifier 'expectedA'
    expectedA = half4(0.75, 0.0, 0.75, 0.25);
    ^^^^^^^^^
error: 6: unknown identifier 'expectedB'
    expectedB = half4(0.25, 0.0, 0.75, 1.0);
    ^^^^^^^^^
error: 9: unknown identifier 'expectedA'
    return (mod(testInputs.x,    1)               == expectedA.x     &&
                                                     ^^^^^^^^^
error: 10: unknown identifier 'expectedA'
            mod(testInputs.xy,   1)               == expectedA.xy    &&
                                                     ^^^^^^^^^
error: 11: unknown identifier 'expectedA'
            mod(testInputs.xyz,  1)               == expectedA.xyz   &&
                                                     ^^^^^^^^^
error: 12: unknown identifier 'expectedA'
            mod(testInputs.xyzw, 1)               == expectedA.xyzw  &&
                                                     ^^^^^^^^^
error: 13: unknown identifier 'expectedA'
            mod(constVal.x,      1)               == expectedA.x     &&
                                                     ^^^^^^^^^
error: 14: unknown identifier 'expectedA'
            mod(constVal.xy,     1)               == expectedA.xy    &&
                                                     ^^^^^^^^^
error: 15: unknown identifier 'expectedA'
            mod(constVal.xyz,    1)               == expectedA.xyz   &&
                                                     ^^^^^^^^^
error: 16: unknown identifier 'expectedA'
            mod(constVal.xyzw,   1)               == expectedA.xyzw  &&
                                                     ^^^^^^^^^
error: 17: unknown identifier 'expectedA'
            mod(testInputs.x,    colorWhite.x)    == expectedA.x     &&
                                                     ^^^^^^^^^
error: 18: unknown identifier 'expectedA'
            mod(testInputs.xy,   colorWhite.xy)   == expectedA.xy    &&
                                                     ^^^^^^^^^
error: 19: unknown identifier 'expectedA'
            mod(testInputs.xyz,  colorWhite.xyz)  == expectedA.xyz   &&
                                                     ^^^^^^^^^
error: 20: unknown identifier 'expectedA'
            mod(testInputs.xyzw, colorWhite.xyzw) == expectedA.xyzw  &&
                                                     ^^^^^^^^^
error: 21: unknown identifier 'expectedB'
            mod(constVal.x,      constRamp.x)     == expectedB.x     &&
                                                     ^^^^^^^^^
error: 22: unknown identifier 'expectedB'
            mod(constVal.xy,     constRamp.xy)    == expectedB.xy    &&
                                                     ^^^^^^^^^
error: 23: unknown identifier 'expectedB'
            mod(constVal.xyz,    constRamp.xyz)   == expectedB.xyz   &&
                                                     ^^^^^^^^^
error: 24: unknown identifier 'expectedB'
            mod(constVal.xyzw,   constRamp.xyzw)  == expectedB.xyzw) ? colorGreen : colorRed;
                                                     ^^^^^^^^^
18 errors
