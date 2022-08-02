### Compilation failed:

error: 6: no match for lessThan(half4, half4)
considered:
$bvec lessThan($uvec x, $uvec y)
$bvec lessThan($usvec x, $usvec y)
$bvec lessThan($svec x, $svec y)

    bool4 expectTTFF = lessThan(half4(1, 2, 2.5, 2.75), half4(2.25));
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: expected 'short4', but found 'int4'
    bool4 expectFFTT = lessThan(int4_zero, int4(-50, 0, 50, 100));
                                ^^^^^^^^^
error: 9: no match for lessThan(half4, half4)
considered:
$bvec lessThan($uvec x, $uvec y)
$bvec lessThan($usvec x, $usvec y)
$bvec lessThan($svec x, $svec y)

    sk_FragColor.x = lessThan(a, b).x ? 1 : 0;
                     ^^^^^^^^^^^^^^
error: 11: expected 'short3', but found 'int3'
    sk_FragColor.z = lessThan(e, f).z ? 1 : 0;
                              ^
error: 12: unknown identifier 'expectTTFF'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                         ^^^^^^^^^^
error: 12: unknown identifier 'expectFFTT'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                                            ^^^^^^^^^^
6 errors
