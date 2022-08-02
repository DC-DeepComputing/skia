### Compilation failed:

error: 6: no match for notEqual(half4, half4)
considered:
$bvec notEqual($usvec x, $usvec y)
$bvec notEqual($svec x, $svec y)
$bvec notEqual($uvec x, $uvec y)

    bool4 expectFFTT = notEqual(half4(3, 3, 3.25, 100), half4(3));
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: expected 'short4', but found 'int4'
    bool4 expectTTFF = notEqual(int4_zero, int4(-100, -50, 0, 0));
                                ^^^^^^^^^
error: 9: no match for notEqual(half4, half4)
considered:
$bvec notEqual($usvec x, $usvec y)
$bvec notEqual($svec x, $svec y)
$bvec notEqual($uvec x, $uvec y)

    sk_FragColor.x = notEqual(a, b).x ? 1 : 0;
                     ^^^^^^^^^^^^^^
error: 11: expected 'short3', but found 'int3'
    sk_FragColor.z = notEqual(e, f).z ? 1 : 0;
                              ^
error: 12: unknown identifier 'expectTTFF'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                         ^^^^^^^^^^
error: 12: unknown identifier 'expectFFTT'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                                            ^^^^^^^^^^
6 errors
