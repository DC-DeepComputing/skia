### Compilation failed:

error: 6: no match for greaterThanEqual(half4, half4)
considered:
$bvec greaterThanEqual($usvec x, $usvec y)
$bvec greaterThanEqual($svec x, $svec y)
$bvec greaterThanEqual($uvec x, $uvec y)

    bool4 expectFFTT = greaterThanEqual(half4(1, 2, 2.25, 2.5), half4(2.25));
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: expected 'short4', but found 'int4'
    bool4 expectTTFF = greaterThanEqual(int4_zero, int4(-50, 0, 50, 100));
                                        ^^^^^^^^^
error: 9: no match for greaterThanEqual(half4, half4)
considered:
$bvec greaterThanEqual($usvec x, $usvec y)
$bvec greaterThanEqual($svec x, $svec y)
$bvec greaterThanEqual($uvec x, $uvec y)

    sk_FragColor.x = greaterThanEqual(a, b).x ? 1 : 0;
                     ^^^^^^^^^^^^^^^^^^^^^^
error: 11: expected 'short3', but found 'int3'
    sk_FragColor.z = greaterThanEqual(e, f).z ? 1 : 0;
                                      ^
error: 12: unknown identifier 'expectTTFF'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                         ^^^^^^^^^^
error: 12: unknown identifier 'expectFFTT'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                                            ^^^^^^^^^^
6 errors
