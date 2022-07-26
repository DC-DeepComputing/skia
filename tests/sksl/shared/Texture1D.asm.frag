### Compilation failed:

error: 1: no type named 'sampler1D'
layout(binding=0) uniform sampler1D tex;
                          ^^^^^^^^^
error: 1: no type named 'tex'
layout(binding=0) uniform sampler1D tex;
                                    ^^^
error: 1: expected a declaration, but found ';'
layout(binding=0) uniform sampler1D tex;
                                       ^
error: 4: unknown identifier 'tex'
    float4 a = sample(tex, 0);
                      ^^^
error: 5: unknown identifier 'tex'
    float4 b = sample(tex, float2(0));
                      ^^^
error: 6: unknown identifier 'a'
    sk_FragColor = half4(float2(a.xy), float2(b.zw));
                                ^
error: 6: unknown identifier 'b'
    sk_FragColor = half4(float2(a.xy), float2(b.zw));
                                              ^
7 errors
