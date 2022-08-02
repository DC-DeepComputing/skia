### Compilation failed:

error: 6: no match for equal(half4, half4)
considered:
$bvec equal($usvec x, $usvec y)
$bvec equal($svec x, $svec y)
$bvec equal($uvec x, $uvec y)

    bool4 expectTTFF = equal(half4(3, 3, 3.25, 100), half4(3));
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: expected 'short4', but found 'int4'
    bool4 expectFFTT = equal(int4_zero, int4(-100, -50, 0, 0));
                             ^^^^^^^^^
error: 8: no match for equal(bool4, bool4)
considered:
$bvec equal($usvec x, $usvec y)
$bvec equal($svec x, $svec y)
$bvec equal($uvec x, $uvec y)

    bool4 expectTTTT = equal(bool4(1), bool4(7));
                       ^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: no match for equal(half4, half4)
considered:
$bvec equal($usvec x, $usvec y)
$bvec equal($svec x, $svec y)
$bvec equal($uvec x, $uvec y)

    sk_FragColor.x = equal(a, b).x ? 1 : 0;
                     ^^^^^^^^^^^
error: 12: expected 'short3', but found 'int3'
    sk_FragColor.z = equal(e, f).z ? 1 : 0;
                           ^
error: 13: unknown identifier 'expectTTFF'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) || any(expectTTTT) ? 1 : 0;
                         ^^^^^^^^^^
error: 13: unknown identifier 'expectFFTT'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) || any(expectTTTT) ? 1 : 0;
                                            ^^^^^^^^^^
error: 13: unknown identifier 'expectTTTT'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) || any(expectTTTT) ? 1 : 0;
                                                               ^^^^^^^^^^
8 errors
