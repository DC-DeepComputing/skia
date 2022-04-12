OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %returns_float2_f2 "returns_float2_f2"
OpName %returns_float3_f3 "returns_float3_f3"
OpName %returns_float4_f4 "returns_float4_f4"
OpName %returns_float2x2_f22 "returns_float2x2_f22"
OpName %returns_float3x3_f33 "returns_float3x3_f33"
OpName %returns_float4x4_f44 "returns_float4x4_f44"
OpName %returns_half_h "returns_half_h"
OpName %returns_half2_h2 "returns_half2_h2"
OpName %returns_half3_h3 "returns_half3_h3"
OpName %returns_half4_h4 "returns_half4_h4"
OpName %returns_half2x2_h22 "returns_half2x2_h22"
OpName %returns_half3x3_h33 "returns_half3x3_h33"
OpName %returns_half4x4_h44 "returns_half4x4_h44"
OpName %returns_bool_b "returns_bool_b"
OpName %returns_bool2_b2 "returns_bool2_b2"
OpName %returns_bool3_b3 "returns_bool3_b3"
OpName %returns_bool4_b4 "returns_bool4_b4"
OpName %returns_int_i "returns_int_i"
OpName %returns_int2_i2 "returns_int2_i2"
OpName %returns_int3_i3 "returns_int3_i3"
OpName %returns_int4_i4 "returns_int4_i4"
OpName %main "main"
OpName %x1 "x1"
OpName %x2 "x2"
OpName %x3 "x3"
OpName %x4 "x4"
OpName %x5 "x5"
OpName %x6 "x6"
OpName %x7 "x7"
OpName %x8 "x8"
OpName %x9 "x9"
OpName %x10 "x10"
OpName %x11 "x11"
OpName %x12 "x12"
OpName %x13 "x13"
OpName %x14 "x14"
OpName %x15 "x15"
OpName %x16 "x16"
OpName %x17 "x17"
OpName %x18 "x18"
OpName %x19 "x19"
OpName %x20 "x20"
OpName %x21 "x21"
OpName %x22 "x22"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %31 Binding 0
OpDecorate %31 DescriptorSet 0
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %x8 RelaxedPrecision
OpDecorate %x9 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%31 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%36 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%40 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%44 = OpTypeFunction %v2float
%float_2 = OpConstant %float 2
%47 = OpConstantComposite %v2float %float_2 %float_2
%v3float = OpTypeVector %float 3
%49 = OpTypeFunction %v3float
%float_3 = OpConstant %float 3
%52 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%53 = OpTypeFunction %v4float
%float_4 = OpConstant %float 4
%56 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%58 = OpTypeFunction %mat2v2float
%61 = OpConstantComposite %v2float %float_2 %float_0
%62 = OpConstantComposite %v2float %float_0 %float_2
%mat3v3float = OpTypeMatrix %v3float 3
%64 = OpTypeFunction %mat3v3float
%67 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%68 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%69 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%mat4v4float = OpTypeMatrix %v4float 4
%71 = OpTypeFunction %mat4v4float
%74 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%75 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%76 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%77 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%78 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%82 = OpConstantComposite %v2float %float_2 %float_2
%84 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%86 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%89 = OpConstantComposite %v2float %float_2 %float_0
%90 = OpConstantComposite %v2float %float_0 %float_2
%93 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%94 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%95 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%98 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%99 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%100 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%101 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%102 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%106 = OpTypeFunction %v2bool
%108 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%110 = OpTypeFunction %v3bool
%112 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%114 = OpTypeFunction %v4bool
%116 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%118 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%122 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%125 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%127 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%130 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%132 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%135 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%136 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_v4int = OpTypePointer Function %v4int
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %36
%37 = OpLabel
%41 = OpVariable %_ptr_Function_v2float Function
OpStore %41 %40
%43 = OpFunctionCall %v4float %main %41
OpStore %sk_FragColor %43
OpReturn
OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %44
%45 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %49
%50 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %53
%54 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %58
%59 = OpLabel
%60 = OpCompositeConstruct %mat2v2float %61 %62
OpReturnValue %60
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %64
%65 = OpLabel
%66 = OpCompositeConstruct %mat3v3float %67 %68 %69
OpReturnValue %66
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %71
%72 = OpLabel
%73 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
OpReturnValue %73
OpFunctionEnd
%returns_half_h = OpFunction %float None %78
%79 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %44
%81 = OpLabel
OpReturnValue %82
OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %49
%83 = OpLabel
OpReturnValue %84
OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %53
%85 = OpLabel
OpReturnValue %86
OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %58
%87 = OpLabel
%88 = OpCompositeConstruct %mat2v2float %89 %90
OpReturnValue %88
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %64
%91 = OpLabel
%92 = OpCompositeConstruct %mat3v3float %93 %94 %95
OpReturnValue %92
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %71
%96 = OpLabel
%97 = OpCompositeConstruct %mat4v4float %98 %99 %100 %101
OpReturnValue %97
OpFunctionEnd
%returns_bool_b = OpFunction %bool None %102
%103 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %106
%107 = OpLabel
OpReturnValue %108
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %110
%111 = OpLabel
OpReturnValue %112
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %114
%115 = OpLabel
OpReturnValue %116
OpFunctionEnd
%returns_int_i = OpFunction %int None %118
%119 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %122
%123 = OpLabel
OpReturnValue %125
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %127
%128 = OpLabel
OpReturnValue %130
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %132
%133 = OpLabel
OpReturnValue %135
OpFunctionEnd
%main = OpFunction %v4float None %136
%137 = OpFunctionParameter %_ptr_Function_v2float
%138 = OpLabel
%x1 = OpVariable %_ptr_Function_float Function
%x2 = OpVariable %_ptr_Function_v2float Function
%x3 = OpVariable %_ptr_Function_v3float Function
%x4 = OpVariable %_ptr_Function_v4float Function
%x5 = OpVariable %_ptr_Function_mat2v2float Function
%x6 = OpVariable %_ptr_Function_mat3v3float Function
%x7 = OpVariable %_ptr_Function_mat4v4float Function
%x8 = OpVariable %_ptr_Function_float Function
%x9 = OpVariable %_ptr_Function_v2float Function
%x10 = OpVariable %_ptr_Function_v3float Function
%x11 = OpVariable %_ptr_Function_v4float Function
%x12 = OpVariable %_ptr_Function_mat2v2float Function
%x13 = OpVariable %_ptr_Function_mat3v3float Function
%x14 = OpVariable %_ptr_Function_mat4v4float Function
%x15 = OpVariable %_ptr_Function_bool Function
%x16 = OpVariable %_ptr_Function_v2bool Function
%x17 = OpVariable %_ptr_Function_v3bool Function
%x18 = OpVariable %_ptr_Function_v4bool Function
%x19 = OpVariable %_ptr_Function_int Function
%x20 = OpVariable %_ptr_Function_v2int Function
%x21 = OpVariable %_ptr_Function_v3int Function
%x22 = OpVariable %_ptr_Function_v4int Function
%400 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %47
OpStore %x3 %52
OpStore %x4 %56
%148 = OpCompositeConstruct %mat2v2float %61 %62
OpStore %x5 %148
%151 = OpCompositeConstruct %mat3v3float %67 %68 %69
OpStore %x6 %151
%154 = OpCompositeConstruct %mat4v4float %74 %75 %76 %77
OpStore %x7 %154
OpStore %x8 %float_1
OpStore %x9 %82
OpStore %x10 %84
OpStore %x11 %86
%160 = OpCompositeConstruct %mat2v2float %89 %90
OpStore %x12 %160
%162 = OpCompositeConstruct %mat3v3float %93 %94 %95
OpStore %x13 %162
%164 = OpCompositeConstruct %mat4v4float %98 %99 %100 %101
OpStore %x14 %164
OpStore %x15 %true
OpStore %x16 %108
OpStore %x17 %112
OpStore %x18 %116
OpStore %x19 %int_1
OpStore %x20 %125
OpStore %x21 %130
OpStore %x22 %135
%182 = OpLoad %float %x1
%183 = OpFOrdEqual %bool %182 %float_1
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %v2float %x2
%187 = OpFunctionCall %v2float %returns_float2_f2
%188 = OpFOrdEqual %v2bool %186 %187
%189 = OpAll %bool %188
OpBranch %185
%185 = OpLabel
%190 = OpPhi %bool %false %138 %189 %184
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpLoad %v3float %x3
%194 = OpFunctionCall %v3float %returns_float3_f3
%195 = OpFOrdEqual %v3bool %193 %194
%196 = OpAll %bool %195
OpBranch %192
%192 = OpLabel
%197 = OpPhi %bool %false %185 %196 %191
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %v4float %x4
%201 = OpFunctionCall %v4float %returns_float4_f4
%202 = OpFOrdEqual %v4bool %200 %201
%203 = OpAll %bool %202
OpBranch %199
%199 = OpLabel
%204 = OpPhi %bool %false %192 %203 %198
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%207 = OpLoad %mat2v2float %x5
%208 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%209 = OpCompositeExtract %v2float %207 0
%210 = OpCompositeExtract %v2float %208 0
%211 = OpFOrdEqual %v2bool %209 %210
%212 = OpAll %bool %211
%213 = OpCompositeExtract %v2float %207 1
%214 = OpCompositeExtract %v2float %208 1
%215 = OpFOrdEqual %v2bool %213 %214
%216 = OpAll %bool %215
%217 = OpLogicalAnd %bool %212 %216
OpBranch %206
%206 = OpLabel
%218 = OpPhi %bool %false %199 %217 %205
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpLoad %mat3v3float %x6
%222 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%223 = OpCompositeExtract %v3float %221 0
%224 = OpCompositeExtract %v3float %222 0
%225 = OpFOrdEqual %v3bool %223 %224
%226 = OpAll %bool %225
%227 = OpCompositeExtract %v3float %221 1
%228 = OpCompositeExtract %v3float %222 1
%229 = OpFOrdEqual %v3bool %227 %228
%230 = OpAll %bool %229
%231 = OpLogicalAnd %bool %226 %230
%232 = OpCompositeExtract %v3float %221 2
%233 = OpCompositeExtract %v3float %222 2
%234 = OpFOrdEqual %v3bool %232 %233
%235 = OpAll %bool %234
%236 = OpLogicalAnd %bool %231 %235
OpBranch %220
%220 = OpLabel
%237 = OpPhi %bool %false %206 %236 %219
OpSelectionMerge %239 None
OpBranchConditional %237 %238 %239
%238 = OpLabel
%240 = OpLoad %mat4v4float %x7
%241 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%242 = OpCompositeExtract %v4float %240 0
%243 = OpCompositeExtract %v4float %241 0
%244 = OpFOrdEqual %v4bool %242 %243
%245 = OpAll %bool %244
%246 = OpCompositeExtract %v4float %240 1
%247 = OpCompositeExtract %v4float %241 1
%248 = OpFOrdEqual %v4bool %246 %247
%249 = OpAll %bool %248
%250 = OpLogicalAnd %bool %245 %249
%251 = OpCompositeExtract %v4float %240 2
%252 = OpCompositeExtract %v4float %241 2
%253 = OpFOrdEqual %v4bool %251 %252
%254 = OpAll %bool %253
%255 = OpLogicalAnd %bool %250 %254
%256 = OpCompositeExtract %v4float %240 3
%257 = OpCompositeExtract %v4float %241 3
%258 = OpFOrdEqual %v4bool %256 %257
%259 = OpAll %bool %258
%260 = OpLogicalAnd %bool %255 %259
OpBranch %239
%239 = OpLabel
%261 = OpPhi %bool %false %220 %260 %238
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpLoad %float %x8
%265 = OpFunctionCall %float %returns_half_h
%266 = OpFOrdEqual %bool %264 %265
OpBranch %263
%263 = OpLabel
%267 = OpPhi %bool %false %239 %266 %262
OpSelectionMerge %269 None
OpBranchConditional %267 %268 %269
%268 = OpLabel
%270 = OpLoad %v2float %x9
%271 = OpFunctionCall %v2float %returns_half2_h2
%272 = OpFOrdEqual %v2bool %270 %271
%273 = OpAll %bool %272
OpBranch %269
%269 = OpLabel
%274 = OpPhi %bool %false %263 %273 %268
OpSelectionMerge %276 None
OpBranchConditional %274 %275 %276
%275 = OpLabel
%277 = OpLoad %v3float %x10
%278 = OpFunctionCall %v3float %returns_half3_h3
%279 = OpFOrdEqual %v3bool %277 %278
%280 = OpAll %bool %279
OpBranch %276
%276 = OpLabel
%281 = OpPhi %bool %false %269 %280 %275
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpLoad %v4float %x11
%285 = OpFunctionCall %v4float %returns_half4_h4
%286 = OpFOrdEqual %v4bool %284 %285
%287 = OpAll %bool %286
OpBranch %283
%283 = OpLabel
%288 = OpPhi %bool %false %276 %287 %282
OpSelectionMerge %290 None
OpBranchConditional %288 %289 %290
%289 = OpLabel
%291 = OpLoad %mat2v2float %x12
%292 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%293 = OpCompositeExtract %v2float %291 0
%294 = OpCompositeExtract %v2float %292 0
%295 = OpFOrdEqual %v2bool %293 %294
%296 = OpAll %bool %295
%297 = OpCompositeExtract %v2float %291 1
%298 = OpCompositeExtract %v2float %292 1
%299 = OpFOrdEqual %v2bool %297 %298
%300 = OpAll %bool %299
%301 = OpLogicalAnd %bool %296 %300
OpBranch %290
%290 = OpLabel
%302 = OpPhi %bool %false %283 %301 %289
OpSelectionMerge %304 None
OpBranchConditional %302 %303 %304
%303 = OpLabel
%305 = OpLoad %mat3v3float %x13
%306 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%307 = OpCompositeExtract %v3float %305 0
%308 = OpCompositeExtract %v3float %306 0
%309 = OpFOrdEqual %v3bool %307 %308
%310 = OpAll %bool %309
%311 = OpCompositeExtract %v3float %305 1
%312 = OpCompositeExtract %v3float %306 1
%313 = OpFOrdEqual %v3bool %311 %312
%314 = OpAll %bool %313
%315 = OpLogicalAnd %bool %310 %314
%316 = OpCompositeExtract %v3float %305 2
%317 = OpCompositeExtract %v3float %306 2
%318 = OpFOrdEqual %v3bool %316 %317
%319 = OpAll %bool %318
%320 = OpLogicalAnd %bool %315 %319
OpBranch %304
%304 = OpLabel
%321 = OpPhi %bool %false %290 %320 %303
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpLoad %mat4v4float %x14
%325 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%326 = OpCompositeExtract %v4float %324 0
%327 = OpCompositeExtract %v4float %325 0
%328 = OpFOrdEqual %v4bool %326 %327
%329 = OpAll %bool %328
%330 = OpCompositeExtract %v4float %324 1
%331 = OpCompositeExtract %v4float %325 1
%332 = OpFOrdEqual %v4bool %330 %331
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %329 %333
%335 = OpCompositeExtract %v4float %324 2
%336 = OpCompositeExtract %v4float %325 2
%337 = OpFOrdEqual %v4bool %335 %336
%338 = OpAll %bool %337
%339 = OpLogicalAnd %bool %334 %338
%340 = OpCompositeExtract %v4float %324 3
%341 = OpCompositeExtract %v4float %325 3
%342 = OpFOrdEqual %v4bool %340 %341
%343 = OpAll %bool %342
%344 = OpLogicalAnd %bool %339 %343
OpBranch %323
%323 = OpLabel
%345 = OpPhi %bool %false %304 %344 %322
OpSelectionMerge %347 None
OpBranchConditional %345 %346 %347
%346 = OpLabel
%348 = OpLoad %bool %x15
%349 = OpFunctionCall %bool %returns_bool_b
%350 = OpLogicalEqual %bool %348 %349
OpBranch %347
%347 = OpLabel
%351 = OpPhi %bool %false %323 %350 %346
OpSelectionMerge %353 None
OpBranchConditional %351 %352 %353
%352 = OpLabel
%354 = OpLoad %v2bool %x16
%355 = OpFunctionCall %v2bool %returns_bool2_b2
%356 = OpLogicalEqual %v2bool %354 %355
%357 = OpAll %bool %356
OpBranch %353
%353 = OpLabel
%358 = OpPhi %bool %false %347 %357 %352
OpSelectionMerge %360 None
OpBranchConditional %358 %359 %360
%359 = OpLabel
%361 = OpLoad %v3bool %x17
%362 = OpFunctionCall %v3bool %returns_bool3_b3
%363 = OpLogicalEqual %v3bool %361 %362
%364 = OpAll %bool %363
OpBranch %360
%360 = OpLabel
%365 = OpPhi %bool %false %353 %364 %359
OpSelectionMerge %367 None
OpBranchConditional %365 %366 %367
%366 = OpLabel
%368 = OpLoad %v4bool %x18
%369 = OpFunctionCall %v4bool %returns_bool4_b4
%370 = OpLogicalEqual %v4bool %368 %369
%371 = OpAll %bool %370
OpBranch %367
%367 = OpLabel
%372 = OpPhi %bool %false %360 %371 %366
OpSelectionMerge %374 None
OpBranchConditional %372 %373 %374
%373 = OpLabel
%375 = OpLoad %int %x19
%376 = OpFunctionCall %int %returns_int_i
%377 = OpIEqual %bool %375 %376
OpBranch %374
%374 = OpLabel
%378 = OpPhi %bool %false %367 %377 %373
OpSelectionMerge %380 None
OpBranchConditional %378 %379 %380
%379 = OpLabel
%381 = OpLoad %v2int %x20
%382 = OpFunctionCall %v2int %returns_int2_i2
%383 = OpIEqual %v2bool %381 %382
%384 = OpAll %bool %383
OpBranch %380
%380 = OpLabel
%385 = OpPhi %bool %false %374 %384 %379
OpSelectionMerge %387 None
OpBranchConditional %385 %386 %387
%386 = OpLabel
%388 = OpLoad %v3int %x21
%389 = OpFunctionCall %v3int %returns_int3_i3
%390 = OpIEqual %v3bool %388 %389
%391 = OpAll %bool %390
OpBranch %387
%387 = OpLabel
%392 = OpPhi %bool %false %380 %391 %386
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%395 = OpLoad %v4int %x22
%396 = OpFunctionCall %v4int %returns_int4_i4
%397 = OpIEqual %v4bool %395 %396
%398 = OpAll %bool %397
OpBranch %394
%394 = OpLabel
%399 = OpPhi %bool %false %387 %398 %393
OpSelectionMerge %403 None
OpBranchConditional %399 %401 %402
%401 = OpLabel
%404 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%407 = OpLoad %v4float %404
OpStore %400 %407
OpBranch %403
%402 = OpLabel
%408 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%409 = OpLoad %v4float %408
OpStore %400 %409
OpBranch %403
%403 = OpLabel
%410 = OpLoad %v4float %400
OpReturnValue %410
OpFunctionEnd
