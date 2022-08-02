### Compilation failed:

error: 6: no match for greaterThan(half4, half4)
considered:
$bvec greaterThan($usvec x, $usvec y)
$bvec greaterThan($svec x, $svec y)
$bvec greaterThan($uvec x, $uvec y)

    bool4 expectFFTT = greaterThan(half4(1, 2, 2.5, 2.75), half4(2.25));
                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: expected 'short4', but found 'int4'
    bool4 expectTTFF = greaterThan(int4_zero, int4(-100, -50, 0, 50));
                                   ^^^^^^^^^
error: 9: no match for greaterThan(half4, half4)
considered:
$bvec greaterThan($usvec x, $usvec y)
$bvec greaterThan($svec x, $svec y)
$bvec greaterThan($uvec x, $uvec y)

    sk_FragColor.x = greaterThan(a, b).x ? 1 : 0;
                     ^^^^^^^^^^^^^^^^^
error: 11: expected 'short3', but found 'int3'
    sk_FragColor.z = greaterThan(e, f).z ? 1 : 0;
                                 ^
error: 12: unknown identifier 'expectTTFF'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                         ^^^^^^^^^^
error: 12: unknown identifier 'expectFFTT'
    sk_FragColor.w = any(expectTTFF) || any(expectFFTT) ? 1 : 0;
                                            ^^^^^^^^^^
6 errors
