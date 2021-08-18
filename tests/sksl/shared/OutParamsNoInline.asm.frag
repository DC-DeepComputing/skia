OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %out_half_vh "out_half_vh"
OpName %out_half2_vh2 "out_half2_vh2"
OpName %out_half3_vh3 "out_half3_vh3"
OpName %out_half4_vh4 "out_half4_vh4"
OpName %out_half2x2_vh22 "out_half2x2_vh22"
OpName %out_half3x3_vh33 "out_half3x3_vh33"
OpName %out_half4x4_vh44 "out_half4x4_vh44"
OpName %out_int_vi "out_int_vi"
OpName %out_int2_vi2 "out_int2_vi2"
OpName %out_int3_vi3 "out_int3_vi3"
OpName %out_int4_vi4 "out_int4_vi4"
OpName %out_float_vf "out_float_vf"
OpName %out_float2_vf2 "out_float2_vf2"
OpName %out_float3_vf3 "out_float3_vf3"
OpName %out_float4_vf4 "out_float4_vf4"
OpName %out_float2x2_vf22 "out_float2x2_vf22"
OpName %out_float3x3_vf33 "out_float3x3_vf33"
OpName %out_float4x4_vf44 "out_float4x4_vf44"
OpName %out_bool_vb "out_bool_vb"
OpName %out_bool2_vb2 "out_bool2_vb2"
OpName %out_bool3_vb3 "out_bool3_vb3"
OpName %out_bool4_vb4 "out_bool4_vb4"
OpName %main "main"
OpName %h "h"
OpName %h2 "h2"
OpName %h3 "h3"
OpName %h4 "h4"
OpName %h2x2 "h2x2"
OpName %h3x3 "h3x3"
OpName %h4x4 "h4x4"
OpName %i "i"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpName %f "f"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %f2x2 "f2x2"
OpName %f3x3 "f3x3"
OpName %f4x4 "f4x4"
OpName %b "b"
OpName %b2 "b2"
OpName %b3 "b3"
OpName %b4 "b4"
OpName %ok "ok"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %34 Binding 0
OpDecorate %34 DescriptorSet 0
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %h RelaxedPrecision
OpDecorate %h2 RelaxedPrecision
OpDecorate %h3 RelaxedPrecision
OpDecorate %h4 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %h2x2 RelaxedPrecision
OpDecorate %h3x3 RelaxedPrecision
OpDecorate %h4x4 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %473 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%34 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%39 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%43 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%47 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%57 = OpTypeFunction %void %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%65 = OpTypeFunction %void %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%73 = OpTypeFunction %void %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%82 = OpTypeFunction %void %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%93 = OpTypeFunction %void %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%105 = OpTypeFunction %void %_ptr_Function_mat4v4float
%_ptr_Function_int = OpTypePointer Function %int
%117 = OpTypeFunction %void %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%126 = OpTypeFunction %void %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%136 = OpTypeFunction %void %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%146 = OpTypeFunction %void %_ptr_Function_v4int
%_ptr_Function_bool = OpTypePointer Function %bool
%205 = OpTypeFunction %void %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%214 = OpTypeFunction %void %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%224 = OpTypeFunction %void %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%234 = OpTypeFunction %void %_ptr_Function_v4bool
%243 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %39
%40 = OpLabel
%44 = OpVariable %_ptr_Function_v2float Function
OpStore %44 %43
%46 = OpFunctionCall %v4float %main %44
OpStore %sk_FragColor %46
OpReturn
OpFunctionEnd
%out_half_vh = OpFunction %void None %47
%49 = OpFunctionParameter %_ptr_Function_float
%50 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%55 = OpLoad %v4float %51
%56 = OpCompositeExtract %float %55 0
OpStore %49 %56
OpReturn
OpFunctionEnd
%out_half2_vh2 = OpFunction %void None %57
%58 = OpFunctionParameter %_ptr_Function_v2float
%59 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%61 = OpLoad %v4float %60
%62 = OpCompositeExtract %float %61 1
%63 = OpCompositeConstruct %v2float %62 %62
OpStore %58 %63
OpReturn
OpFunctionEnd
%out_half3_vh3 = OpFunction %void None %65
%67 = OpFunctionParameter %_ptr_Function_v3float
%68 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%70 = OpLoad %v4float %69
%71 = OpCompositeExtract %float %70 2
%72 = OpCompositeConstruct %v3float %71 %71 %71
OpStore %67 %72
OpReturn
OpFunctionEnd
%out_half4_vh4 = OpFunction %void None %73
%75 = OpFunctionParameter %_ptr_Function_v4float
%76 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%78 = OpLoad %v4float %77
%79 = OpCompositeExtract %float %78 3
%80 = OpCompositeConstruct %v4float %79 %79 %79 %79
OpStore %75 %80
OpReturn
OpFunctionEnd
%out_half2x2_vh22 = OpFunction %void None %82
%84 = OpFunctionParameter %_ptr_Function_mat2v2float
%85 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%87 = OpLoad %v4float %86
%88 = OpCompositeExtract %float %87 0
%90 = OpCompositeConstruct %v2float %88 %float_0
%91 = OpCompositeConstruct %v2float %float_0 %88
%89 = OpCompositeConstruct %mat2v2float %90 %91
OpStore %84 %89
OpReturn
OpFunctionEnd
%out_half3x3_vh33 = OpFunction %void None %93
%95 = OpFunctionParameter %_ptr_Function_mat3v3float
%96 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%98 = OpLoad %v4float %97
%99 = OpCompositeExtract %float %98 1
%101 = OpCompositeConstruct %v3float %99 %float_0 %float_0
%102 = OpCompositeConstruct %v3float %float_0 %99 %float_0
%103 = OpCompositeConstruct %v3float %float_0 %float_0 %99
%100 = OpCompositeConstruct %mat3v3float %101 %102 %103
OpStore %95 %100
OpReturn
OpFunctionEnd
%out_half4x4_vh44 = OpFunction %void None %105
%107 = OpFunctionParameter %_ptr_Function_mat4v4float
%108 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%110 = OpLoad %v4float %109
%111 = OpCompositeExtract %float %110 2
%113 = OpCompositeConstruct %v4float %111 %float_0 %float_0 %float_0
%114 = OpCompositeConstruct %v4float %float_0 %111 %float_0 %float_0
%115 = OpCompositeConstruct %v4float %float_0 %float_0 %111 %float_0
%116 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %111
%112 = OpCompositeConstruct %mat4v4float %113 %114 %115 %116
OpStore %107 %112
OpReturn
OpFunctionEnd
%out_int_vi = OpFunction %void None %117
%119 = OpFunctionParameter %_ptr_Function_int
%120 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%122 = OpLoad %v4float %121
%123 = OpCompositeExtract %float %122 0
%124 = OpConvertFToS %int %123
OpStore %119 %124
OpReturn
OpFunctionEnd
%out_int2_vi2 = OpFunction %void None %126
%128 = OpFunctionParameter %_ptr_Function_v2int
%129 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 1
%133 = OpConvertFToS %int %132
%134 = OpCompositeConstruct %v2int %133 %133
OpStore %128 %134
OpReturn
OpFunctionEnd
%out_int3_vi3 = OpFunction %void None %136
%138 = OpFunctionParameter %_ptr_Function_v3int
%139 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%141 = OpLoad %v4float %140
%142 = OpCompositeExtract %float %141 2
%143 = OpConvertFToS %int %142
%144 = OpCompositeConstruct %v3int %143 %143 %143
OpStore %138 %144
OpReturn
OpFunctionEnd
%out_int4_vi4 = OpFunction %void None %146
%148 = OpFunctionParameter %_ptr_Function_v4int
%149 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%151 = OpLoad %v4float %150
%152 = OpCompositeExtract %float %151 3
%153 = OpConvertFToS %int %152
%154 = OpCompositeConstruct %v4int %153 %153 %153 %153
OpStore %148 %154
OpReturn
OpFunctionEnd
%out_float_vf = OpFunction %void None %47
%155 = OpFunctionParameter %_ptr_Function_float
%156 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%158 = OpLoad %v4float %157
%159 = OpCompositeExtract %float %158 0
OpStore %155 %159
OpReturn
OpFunctionEnd
%out_float2_vf2 = OpFunction %void None %57
%160 = OpFunctionParameter %_ptr_Function_v2float
%161 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%163 = OpLoad %v4float %162
%164 = OpCompositeExtract %float %163 1
%165 = OpCompositeConstruct %v2float %164 %164
OpStore %160 %165
OpReturn
OpFunctionEnd
%out_float3_vf3 = OpFunction %void None %65
%166 = OpFunctionParameter %_ptr_Function_v3float
%167 = OpLabel
%168 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%169 = OpLoad %v4float %168
%170 = OpCompositeExtract %float %169 2
%171 = OpCompositeConstruct %v3float %170 %170 %170
OpStore %166 %171
OpReturn
OpFunctionEnd
%out_float4_vf4 = OpFunction %void None %73
%172 = OpFunctionParameter %_ptr_Function_v4float
%173 = OpLabel
%174 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%175 = OpLoad %v4float %174
%176 = OpCompositeExtract %float %175 3
%177 = OpCompositeConstruct %v4float %176 %176 %176 %176
OpStore %172 %177
OpReturn
OpFunctionEnd
%out_float2x2_vf22 = OpFunction %void None %82
%178 = OpFunctionParameter %_ptr_Function_mat2v2float
%179 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%181 = OpLoad %v4float %180
%182 = OpCompositeExtract %float %181 0
%184 = OpCompositeConstruct %v2float %182 %float_0
%185 = OpCompositeConstruct %v2float %float_0 %182
%183 = OpCompositeConstruct %mat2v2float %184 %185
OpStore %178 %183
OpReturn
OpFunctionEnd
%out_float3x3_vf33 = OpFunction %void None %93
%186 = OpFunctionParameter %_ptr_Function_mat3v3float
%187 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%189 = OpLoad %v4float %188
%190 = OpCompositeExtract %float %189 1
%192 = OpCompositeConstruct %v3float %190 %float_0 %float_0
%193 = OpCompositeConstruct %v3float %float_0 %190 %float_0
%194 = OpCompositeConstruct %v3float %float_0 %float_0 %190
%191 = OpCompositeConstruct %mat3v3float %192 %193 %194
OpStore %186 %191
OpReturn
OpFunctionEnd
%out_float4x4_vf44 = OpFunction %void None %105
%195 = OpFunctionParameter %_ptr_Function_mat4v4float
%196 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%198 = OpLoad %v4float %197
%199 = OpCompositeExtract %float %198 2
%201 = OpCompositeConstruct %v4float %199 %float_0 %float_0 %float_0
%202 = OpCompositeConstruct %v4float %float_0 %199 %float_0 %float_0
%203 = OpCompositeConstruct %v4float %float_0 %float_0 %199 %float_0
%204 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %199
%200 = OpCompositeConstruct %mat4v4float %201 %202 %203 %204
OpStore %195 %200
OpReturn
OpFunctionEnd
%out_bool_vb = OpFunction %void None %205
%207 = OpFunctionParameter %_ptr_Function_bool
%208 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%210 = OpLoad %v4float %209
%211 = OpCompositeExtract %float %210 0
%212 = OpFUnordNotEqual %bool %211 %float_0
OpStore %207 %212
OpReturn
OpFunctionEnd
%out_bool2_vb2 = OpFunction %void None %214
%216 = OpFunctionParameter %_ptr_Function_v2bool
%217 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%219 = OpLoad %v4float %218
%220 = OpCompositeExtract %float %219 1
%221 = OpFUnordNotEqual %bool %220 %float_0
%222 = OpCompositeConstruct %v2bool %221 %221
OpStore %216 %222
OpReturn
OpFunctionEnd
%out_bool3_vb3 = OpFunction %void None %224
%226 = OpFunctionParameter %_ptr_Function_v3bool
%227 = OpLabel
%228 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%229 = OpLoad %v4float %228
%230 = OpCompositeExtract %float %229 2
%231 = OpFUnordNotEqual %bool %230 %float_0
%232 = OpCompositeConstruct %v3bool %231 %231 %231
OpStore %226 %232
OpReturn
OpFunctionEnd
%out_bool4_vb4 = OpFunction %void None %234
%236 = OpFunctionParameter %_ptr_Function_v4bool
%237 = OpLabel
%238 = OpAccessChain %_ptr_Uniform_v4float %34 %int_2
%239 = OpLoad %v4float %238
%240 = OpCompositeExtract %float %239 3
%241 = OpFUnordNotEqual %bool %240 %float_0
%242 = OpCompositeConstruct %v4bool %241 %241 %241 %241
OpStore %236 %242
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %243
%244 = OpFunctionParameter %_ptr_Function_v2float
%245 = OpLabel
%h = OpVariable %_ptr_Function_float Function
%h2 = OpVariable %_ptr_Function_v2float Function
%h3 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%257 = OpVariable %_ptr_Function_float Function
%262 = OpVariable %_ptr_Function_v2float Function
%269 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%282 = OpVariable %_ptr_Function_v3float Function
%289 = OpVariable %_ptr_Function_float Function
%296 = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_v2int Function
%i3 = OpVariable %_ptr_Function_v3int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%309 = OpVariable %_ptr_Function_v3int Function
%316 = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f4 = OpVariable %_ptr_Function_v4float Function
%329 = OpVariable %_ptr_Function_v2float Function
%336 = OpVariable %_ptr_Function_float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%f4x4 = OpVariable %_ptr_Function_mat4v4float Function
%348 = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_v2bool Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%361 = OpVariable %_ptr_Function_v2bool Function
%368 = OpVariable %_ptr_Function_bool Function
%ok = OpVariable %_ptr_Function_bool Function
%465 = OpVariable %_ptr_Function_v4float Function
%247 = OpFunctionCall %void %out_half_vh %h
%249 = OpFunctionCall %void %out_half2_vh2 %h2
%251 = OpFunctionCall %void %out_half3_vh3 %h3
%253 = OpFunctionCall %void %out_half4_vh4 %h4
%254 = OpAccessChain %_ptr_Function_float %h3 %int_1
%256 = OpLoad %float %254
OpStore %257 %256
%258 = OpFunctionCall %void %out_half_vh %257
%259 = OpLoad %float %257
OpStore %254 %259
%260 = OpLoad %v3float %h3
%261 = OpVectorShuffle %v2float %260 %260 0 2
OpStore %262 %261
%263 = OpFunctionCall %void %out_half2_vh2 %262
%264 = OpLoad %v2float %262
%265 = OpLoad %v3float %h3
%266 = OpVectorShuffle %v3float %265 %264 3 1 4
OpStore %h3 %266
%267 = OpLoad %v4float %h4
%268 = OpVectorShuffle %v4float %267 %267 2 3 0 1
OpStore %269 %268
%270 = OpFunctionCall %void %out_half4_vh4 %269
%271 = OpLoad %v4float %269
%272 = OpLoad %v4float %h4
%273 = OpVectorShuffle %v4float %272 %271 6 7 4 5
OpStore %h4 %273
%275 = OpFunctionCall %void %out_half2x2_vh22 %h2x2
%277 = OpFunctionCall %void %out_half3x3_vh33 %h3x3
%279 = OpFunctionCall %void %out_half4x4_vh44 %h4x4
%280 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
%281 = OpLoad %v3float %280
OpStore %282 %281
%283 = OpFunctionCall %void %out_half3_vh3 %282
%284 = OpLoad %v3float %282
OpStore %280 %284
%286 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%287 = OpAccessChain %_ptr_Function_float %286 %int_3
%288 = OpLoad %float %287
OpStore %289 %288
%290 = OpFunctionCall %void %out_half_vh %289
%291 = OpLoad %float %289
OpStore %287 %291
%293 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%294 = OpAccessChain %_ptr_Function_float %293 %int_0
%295 = OpLoad %float %294
OpStore %296 %295
%297 = OpFunctionCall %void %out_half_vh %296
%298 = OpLoad %float %296
OpStore %294 %298
%300 = OpFunctionCall %void %out_int_vi %i
%302 = OpFunctionCall %void %out_int2_vi2 %i2
%304 = OpFunctionCall %void %out_int3_vi3 %i3
%306 = OpFunctionCall %void %out_int4_vi4 %i4
%307 = OpLoad %v4int %i4
%308 = OpVectorShuffle %v3int %307 %307 0 1 2
OpStore %309 %308
%310 = OpFunctionCall %void %out_int3_vi3 %309
%311 = OpLoad %v3int %309
%312 = OpLoad %v4int %i4
%313 = OpVectorShuffle %v4int %312 %311 4 5 6 3
OpStore %i4 %313
%314 = OpAccessChain %_ptr_Function_int %i2 %int_1
%315 = OpLoad %int %314
OpStore %316 %315
%317 = OpFunctionCall %void %out_int_vi %316
%318 = OpLoad %int %316
OpStore %314 %318
%320 = OpFunctionCall %void %out_float_vf %f
%322 = OpFunctionCall %void %out_float2_vf2 %f2
%324 = OpFunctionCall %void %out_float3_vf3 %f3
%326 = OpFunctionCall %void %out_float4_vf4 %f4
%327 = OpLoad %v3float %f3
%328 = OpVectorShuffle %v2float %327 %327 0 1
OpStore %329 %328
%330 = OpFunctionCall %void %out_float2_vf2 %329
%331 = OpLoad %v2float %329
%332 = OpLoad %v3float %f3
%333 = OpVectorShuffle %v3float %332 %331 3 4 2
OpStore %f3 %333
%334 = OpAccessChain %_ptr_Function_float %f2 %int_0
%335 = OpLoad %float %334
OpStore %336 %335
%337 = OpFunctionCall %void %out_float_vf %336
%338 = OpLoad %float %336
OpStore %334 %338
%340 = OpFunctionCall %void %out_float2x2_vf22 %f2x2
%342 = OpFunctionCall %void %out_float3x3_vf33 %f3x3
%344 = OpFunctionCall %void %out_float4x4_vf44 %f4x4
%345 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%346 = OpAccessChain %_ptr_Function_float %345 %int_0
%347 = OpLoad %float %346
OpStore %348 %347
%349 = OpFunctionCall %void %out_float_vf %348
%350 = OpLoad %float %348
OpStore %346 %350
%352 = OpFunctionCall %void %out_bool_vb %b
%354 = OpFunctionCall %void %out_bool2_vb2 %b2
%356 = OpFunctionCall %void %out_bool3_vb3 %b3
%358 = OpFunctionCall %void %out_bool4_vb4 %b4
%359 = OpLoad %v4bool %b4
%360 = OpVectorShuffle %v2bool %359 %359 0 3
OpStore %361 %360
%362 = OpFunctionCall %void %out_bool2_vb2 %361
%363 = OpLoad %v2bool %361
%364 = OpLoad %v4bool %b4
%365 = OpVectorShuffle %v4bool %364 %363 4 1 2 5
OpStore %b4 %365
%366 = OpAccessChain %_ptr_Function_bool %b3 %int_2
%367 = OpLoad %bool %366
OpStore %368 %367
%369 = OpFunctionCall %void %out_bool_vb %368
%370 = OpLoad %bool %368
OpStore %366 %370
OpStore %ok %true
%374 = OpLoad %bool %ok
OpSelectionMerge %376 None
OpBranchConditional %374 %375 %376
%375 = OpLabel
%378 = OpLoad %float %h
%379 = OpLoad %v2float %h2
%380 = OpCompositeExtract %float %379 0
%381 = OpFMul %float %378 %380
%382 = OpLoad %v3float %h3
%383 = OpCompositeExtract %float %382 0
%384 = OpFMul %float %381 %383
%385 = OpLoad %v4float %h4
%386 = OpCompositeExtract %float %385 0
%387 = OpFMul %float %384 %386
%388 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%389 = OpLoad %v2float %388
%390 = OpCompositeExtract %float %389 0
%391 = OpFMul %float %387 %390
%392 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%393 = OpLoad %v3float %392
%394 = OpCompositeExtract %float %393 0
%395 = OpFMul %float %391 %394
%396 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%397 = OpLoad %v4float %396
%398 = OpCompositeExtract %float %397 0
%399 = OpFMul %float %395 %398
%400 = OpFOrdEqual %bool %float_1 %399
OpBranch %376
%376 = OpLabel
%401 = OpPhi %bool %false %245 %400 %375
OpStore %ok %401
%402 = OpLoad %bool %ok
OpSelectionMerge %404 None
OpBranchConditional %402 %403 %404
%403 = OpLabel
%405 = OpLoad %float %f
%406 = OpLoad %v2float %f2
%407 = OpCompositeExtract %float %406 0
%408 = OpFMul %float %405 %407
%409 = OpLoad %v3float %f3
%410 = OpCompositeExtract %float %409 0
%411 = OpFMul %float %408 %410
%412 = OpLoad %v4float %f4
%413 = OpCompositeExtract %float %412 0
%414 = OpFMul %float %411 %413
%415 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%416 = OpLoad %v2float %415
%417 = OpCompositeExtract %float %416 0
%418 = OpFMul %float %414 %417
%419 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%420 = OpLoad %v3float %419
%421 = OpCompositeExtract %float %420 0
%422 = OpFMul %float %418 %421
%423 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%424 = OpLoad %v4float %423
%425 = OpCompositeExtract %float %424 0
%426 = OpFMul %float %422 %425
%427 = OpFOrdEqual %bool %float_1 %426
OpBranch %404
%404 = OpLabel
%428 = OpPhi %bool %false %376 %427 %403
OpStore %ok %428
%429 = OpLoad %bool %ok
OpSelectionMerge %431 None
OpBranchConditional %429 %430 %431
%430 = OpLabel
%432 = OpLoad %int %i
%433 = OpLoad %v2int %i2
%434 = OpCompositeExtract %int %433 0
%435 = OpIMul %int %432 %434
%436 = OpLoad %v3int %i3
%437 = OpCompositeExtract %int %436 0
%438 = OpIMul %int %435 %437
%439 = OpLoad %v4int %i4
%440 = OpCompositeExtract %int %439 0
%441 = OpIMul %int %438 %440
%442 = OpIEqual %bool %int_1 %441
OpBranch %431
%431 = OpLabel
%443 = OpPhi %bool %false %404 %442 %430
OpStore %ok %443
%444 = OpLoad %bool %ok
OpSelectionMerge %446 None
OpBranchConditional %444 %445 %446
%445 = OpLabel
%447 = OpLoad %bool %b
OpSelectionMerge %449 None
OpBranchConditional %447 %448 %449
%448 = OpLabel
%450 = OpLoad %v2bool %b2
%451 = OpCompositeExtract %bool %450 0
OpBranch %449
%449 = OpLabel
%452 = OpPhi %bool %false %445 %451 %448
OpSelectionMerge %454 None
OpBranchConditional %452 %453 %454
%453 = OpLabel
%455 = OpLoad %v3bool %b3
%456 = OpCompositeExtract %bool %455 0
OpBranch %454
%454 = OpLabel
%457 = OpPhi %bool %false %449 %456 %453
OpSelectionMerge %459 None
OpBranchConditional %457 %458 %459
%458 = OpLabel
%460 = OpLoad %v4bool %b4
%461 = OpCompositeExtract %bool %460 0
OpBranch %459
%459 = OpLabel
%462 = OpPhi %bool %false %454 %461 %458
OpBranch %446
%446 = OpLabel
%463 = OpPhi %bool %false %431 %462 %459
OpStore %ok %463
%464 = OpLoad %bool %ok
OpSelectionMerge %468 None
OpBranchConditional %464 %466 %467
%466 = OpLabel
%469 = OpAccessChain %_ptr_Uniform_v4float %34 %int_0
%470 = OpLoad %v4float %469
OpStore %465 %470
OpBranch %468
%467 = OpLabel
%471 = OpAccessChain %_ptr_Uniform_v4float %34 %int_1
%472 = OpLoad %v4float %471
OpStore %465 %472
OpBranch %468
%468 = OpLabel
%473 = OpLoad %v4float %465
OpReturnValue %473
OpFunctionEnd
