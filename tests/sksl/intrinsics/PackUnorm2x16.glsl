### Compilation failed:

error: 13: no match for lessThan(float2, float2)
considered:
$bvec lessThan($uvec x, $uvec y)
$bvec lessThan($usvec x, $usvec y)
$bvec lessThan($svec x, $svec y)

    return (all(lessThan(abs(unpackUnorm2x16(xy)      - float2(expected.xy)), tolerance)) &&
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 14: no match for lessThan(float2, float2)
considered:
$bvec lessThan($uvec x, $uvec y)
$bvec lessThan($usvec x, $usvec y)
$bvec lessThan($svec x, $svec y)

            all(lessThan(abs(unpackUnorm2x16(zw)      - float2(expected.zw)), tolerance)) &&
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: no match for lessThan(float2, float2)
considered:
$bvec lessThan($uvec x, $uvec y)
$bvec lessThan($usvec x, $usvec y)
$bvec lessThan($svec x, $svec y)

            all(lessThan(abs(unpackUnorm2x16(constXY) - float2(expected.xy)), tolerance)) &&
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: no match for lessThan(float2, float2)
considered:
$bvec lessThan($uvec x, $uvec y)
$bvec lessThan($usvec x, $usvec y)
$bvec lessThan($svec x, $svec y)

            all(lessThan(abs(unpackUnorm2x16(constZW) - float2(expected.zw)), tolerance)))
                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
4 errors
