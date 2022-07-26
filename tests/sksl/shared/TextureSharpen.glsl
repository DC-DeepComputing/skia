### Compilation failed:

error: 3: no type named 'sampler1D'
layout(binding=0) uniform sampler1D one;
                          ^^^^^^^^^
error: 3: no type named 'one'
layout(binding=0) uniform sampler1D one;
                                    ^^^
error: 3: expected a declaration, but found ';'
layout(binding=0) uniform sampler1D one;
                                       ^
error: 7: unknown identifier 'one'
    float4 a = sample(one, 0);
                      ^^^
error: 9: unknown identifier 'one'
    float4 c = sample(one, float2(0));
                      ^^^
error: 11: unknown identifier 'a'
    sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x));
                              ^
error: 11: unknown identifier 'c'
    sk_FragColor = half4(half(a.x), half(b.x), half(c.x), half(d.x));
                                                    ^
7 errors
