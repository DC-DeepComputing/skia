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
OpDecorate %x8 RelaxedPrecision
OpDecorate %x9 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
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
%60 = OpConstantComposite %v2float %float_2 %float_0
%61 = OpConstantComposite %v2float %float_0 %float_2
%62 = OpConstantComposite %mat2v2float %60 %61
%mat3v3float = OpTypeMatrix %v3float 3
%64 = OpTypeFunction %mat3v3float
%66 = OpConstantComposite %v3float %float_3 %float_0 %float_0
%67 = OpConstantComposite %v3float %float_0 %float_3 %float_0
%68 = OpConstantComposite %v3float %float_0 %float_0 %float_3
%69 = OpConstantComposite %mat3v3float %66 %67 %68
%mat4v4float = OpTypeMatrix %v4float 4
%71 = OpTypeFunction %mat4v4float
%73 = OpConstantComposite %v4float %float_4 %float_0 %float_0 %float_0
%74 = OpConstantComposite %v4float %float_0 %float_4 %float_0 %float_0
%75 = OpConstantComposite %v4float %float_0 %float_0 %float_4 %float_0
%76 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_4
%77 = OpConstantComposite %mat4v4float %73 %74 %75 %76
%78 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%87 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%91 = OpTypeFunction %v2bool
%93 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%95 = OpTypeFunction %v3bool
%97 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%99 = OpTypeFunction %v4bool
%101 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%103 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%107 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%110 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%112 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%115 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%117 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%120 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%121 = OpTypeFunction %v4float %_ptr_Function_v2float
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
OpReturnValue %62
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %64
%65 = OpLabel
OpReturnValue %69
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %71
%72 = OpLabel
OpReturnValue %77
OpFunctionEnd
%returns_half_h = OpFunction %float None %78
%79 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %44
%81 = OpLabel
OpReturnValue %47
OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %49
%82 = OpLabel
OpReturnValue %52
OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %53
%83 = OpLabel
OpReturnValue %56
OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %58
%84 = OpLabel
OpReturnValue %62
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %64
%85 = OpLabel
OpReturnValue %69
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %71
%86 = OpLabel
OpReturnValue %77
OpFunctionEnd
%returns_bool_b = OpFunction %bool None %87
%88 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %91
%92 = OpLabel
OpReturnValue %93
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %95
%96 = OpLabel
OpReturnValue %97
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %99
%100 = OpLabel
OpReturnValue %101
OpFunctionEnd
%returns_int_i = OpFunction %int None %103
%104 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %107
%108 = OpLabel
OpReturnValue %110
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %112
%113 = OpLabel
OpReturnValue %115
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %117
%118 = OpLabel
OpReturnValue %120
OpFunctionEnd
%main = OpFunction %v4float None %121
%122 = OpFunctionParameter %_ptr_Function_v2float
%123 = OpLabel
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
%378 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %47
OpStore %x3 %52
OpStore %x4 %56
OpStore %x5 %62
OpStore %x6 %69
OpStore %x7 %77
OpStore %x8 %float_1
OpStore %x9 %47
OpStore %x10 %52
OpStore %x11 %56
OpStore %x12 %62
OpStore %x13 %69
OpStore %x14 %77
OpStore %x15 %true
OpStore %x16 %93
OpStore %x17 %97
OpStore %x18 %101
OpStore %x19 %int_1
OpStore %x20 %110
OpStore %x21 %115
OpStore %x22 %120
%161 = OpFOrdEqual %bool %float_1 %float_1
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %v2float %x2
%165 = OpFunctionCall %v2float %returns_float2_f2
%166 = OpFOrdEqual %v2bool %164 %165
%167 = OpAll %bool %166
OpBranch %163
%163 = OpLabel
%168 = OpPhi %bool %false %123 %167 %162
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpLoad %v3float %x3
%172 = OpFunctionCall %v3float %returns_float3_f3
%173 = OpFOrdEqual %v3bool %171 %172
%174 = OpAll %bool %173
OpBranch %170
%170 = OpLabel
%175 = OpPhi %bool %false %163 %174 %169
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%178 = OpLoad %v4float %x4
%179 = OpFunctionCall %v4float %returns_float4_f4
%180 = OpFOrdEqual %v4bool %178 %179
%181 = OpAll %bool %180
OpBranch %177
%177 = OpLabel
%182 = OpPhi %bool %false %170 %181 %176
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %mat2v2float %x5
%186 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%187 = OpCompositeExtract %v2float %185 0
%188 = OpCompositeExtract %v2float %186 0
%189 = OpFOrdEqual %v2bool %187 %188
%190 = OpAll %bool %189
%191 = OpCompositeExtract %v2float %185 1
%192 = OpCompositeExtract %v2float %186 1
%193 = OpFOrdEqual %v2bool %191 %192
%194 = OpAll %bool %193
%195 = OpLogicalAnd %bool %190 %194
OpBranch %184
%184 = OpLabel
%196 = OpPhi %bool %false %177 %195 %183
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpLoad %mat3v3float %x6
%200 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%201 = OpCompositeExtract %v3float %199 0
%202 = OpCompositeExtract %v3float %200 0
%203 = OpFOrdEqual %v3bool %201 %202
%204 = OpAll %bool %203
%205 = OpCompositeExtract %v3float %199 1
%206 = OpCompositeExtract %v3float %200 1
%207 = OpFOrdEqual %v3bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
%210 = OpCompositeExtract %v3float %199 2
%211 = OpCompositeExtract %v3float %200 2
%212 = OpFOrdEqual %v3bool %210 %211
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %209 %213
OpBranch %198
%198 = OpLabel
%215 = OpPhi %bool %false %184 %214 %197
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%218 = OpLoad %mat4v4float %x7
%219 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%220 = OpCompositeExtract %v4float %218 0
%221 = OpCompositeExtract %v4float %219 0
%222 = OpFOrdEqual %v4bool %220 %221
%223 = OpAll %bool %222
%224 = OpCompositeExtract %v4float %218 1
%225 = OpCompositeExtract %v4float %219 1
%226 = OpFOrdEqual %v4bool %224 %225
%227 = OpAll %bool %226
%228 = OpLogicalAnd %bool %223 %227
%229 = OpCompositeExtract %v4float %218 2
%230 = OpCompositeExtract %v4float %219 2
%231 = OpFOrdEqual %v4bool %229 %230
%232 = OpAll %bool %231
%233 = OpLogicalAnd %bool %228 %232
%234 = OpCompositeExtract %v4float %218 3
%235 = OpCompositeExtract %v4float %219 3
%236 = OpFOrdEqual %v4bool %234 %235
%237 = OpAll %bool %236
%238 = OpLogicalAnd %bool %233 %237
OpBranch %217
%217 = OpLabel
%239 = OpPhi %bool %false %198 %238 %216
OpSelectionMerge %241 None
OpBranchConditional %239 %240 %241
%240 = OpLabel
%242 = OpLoad %float %x8
%243 = OpFunctionCall %float %returns_half_h
%244 = OpFOrdEqual %bool %242 %243
OpBranch %241
%241 = OpLabel
%245 = OpPhi %bool %false %217 %244 %240
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%248 = OpLoad %v2float %x9
%249 = OpFunctionCall %v2float %returns_half2_h2
%250 = OpFOrdEqual %v2bool %248 %249
%251 = OpAll %bool %250
OpBranch %247
%247 = OpLabel
%252 = OpPhi %bool %false %241 %251 %246
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %v3float %x10
%256 = OpFunctionCall %v3float %returns_half3_h3
%257 = OpFOrdEqual %v3bool %255 %256
%258 = OpAll %bool %257
OpBranch %254
%254 = OpLabel
%259 = OpPhi %bool %false %247 %258 %253
OpSelectionMerge %261 None
OpBranchConditional %259 %260 %261
%260 = OpLabel
%262 = OpLoad %v4float %x11
%263 = OpFunctionCall %v4float %returns_half4_h4
%264 = OpFOrdEqual %v4bool %262 %263
%265 = OpAll %bool %264
OpBranch %261
%261 = OpLabel
%266 = OpPhi %bool %false %254 %265 %260
OpSelectionMerge %268 None
OpBranchConditional %266 %267 %268
%267 = OpLabel
%269 = OpLoad %mat2v2float %x12
%270 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%271 = OpCompositeExtract %v2float %269 0
%272 = OpCompositeExtract %v2float %270 0
%273 = OpFOrdEqual %v2bool %271 %272
%274 = OpAll %bool %273
%275 = OpCompositeExtract %v2float %269 1
%276 = OpCompositeExtract %v2float %270 1
%277 = OpFOrdEqual %v2bool %275 %276
%278 = OpAll %bool %277
%279 = OpLogicalAnd %bool %274 %278
OpBranch %268
%268 = OpLabel
%280 = OpPhi %bool %false %261 %279 %267
OpSelectionMerge %282 None
OpBranchConditional %280 %281 %282
%281 = OpLabel
%283 = OpLoad %mat3v3float %x13
%284 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%285 = OpCompositeExtract %v3float %283 0
%286 = OpCompositeExtract %v3float %284 0
%287 = OpFOrdEqual %v3bool %285 %286
%288 = OpAll %bool %287
%289 = OpCompositeExtract %v3float %283 1
%290 = OpCompositeExtract %v3float %284 1
%291 = OpFOrdEqual %v3bool %289 %290
%292 = OpAll %bool %291
%293 = OpLogicalAnd %bool %288 %292
%294 = OpCompositeExtract %v3float %283 2
%295 = OpCompositeExtract %v3float %284 2
%296 = OpFOrdEqual %v3bool %294 %295
%297 = OpAll %bool %296
%298 = OpLogicalAnd %bool %293 %297
OpBranch %282
%282 = OpLabel
%299 = OpPhi %bool %false %268 %298 %281
OpSelectionMerge %301 None
OpBranchConditional %299 %300 %301
%300 = OpLabel
%302 = OpLoad %mat4v4float %x14
%303 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%304 = OpCompositeExtract %v4float %302 0
%305 = OpCompositeExtract %v4float %303 0
%306 = OpFOrdEqual %v4bool %304 %305
%307 = OpAll %bool %306
%308 = OpCompositeExtract %v4float %302 1
%309 = OpCompositeExtract %v4float %303 1
%310 = OpFOrdEqual %v4bool %308 %309
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %307 %311
%313 = OpCompositeExtract %v4float %302 2
%314 = OpCompositeExtract %v4float %303 2
%315 = OpFOrdEqual %v4bool %313 %314
%316 = OpAll %bool %315
%317 = OpLogicalAnd %bool %312 %316
%318 = OpCompositeExtract %v4float %302 3
%319 = OpCompositeExtract %v4float %303 3
%320 = OpFOrdEqual %v4bool %318 %319
%321 = OpAll %bool %320
%322 = OpLogicalAnd %bool %317 %321
OpBranch %301
%301 = OpLabel
%323 = OpPhi %bool %false %282 %322 %300
OpSelectionMerge %325 None
OpBranchConditional %323 %324 %325
%324 = OpLabel
%326 = OpLoad %bool %x15
%327 = OpFunctionCall %bool %returns_bool_b
%328 = OpLogicalEqual %bool %326 %327
OpBranch %325
%325 = OpLabel
%329 = OpPhi %bool %false %301 %328 %324
OpSelectionMerge %331 None
OpBranchConditional %329 %330 %331
%330 = OpLabel
%332 = OpLoad %v2bool %x16
%333 = OpFunctionCall %v2bool %returns_bool2_b2
%334 = OpLogicalEqual %v2bool %332 %333
%335 = OpAll %bool %334
OpBranch %331
%331 = OpLabel
%336 = OpPhi %bool %false %325 %335 %330
OpSelectionMerge %338 None
OpBranchConditional %336 %337 %338
%337 = OpLabel
%339 = OpLoad %v3bool %x17
%340 = OpFunctionCall %v3bool %returns_bool3_b3
%341 = OpLogicalEqual %v3bool %339 %340
%342 = OpAll %bool %341
OpBranch %338
%338 = OpLabel
%343 = OpPhi %bool %false %331 %342 %337
OpSelectionMerge %345 None
OpBranchConditional %343 %344 %345
%344 = OpLabel
%346 = OpLoad %v4bool %x18
%347 = OpFunctionCall %v4bool %returns_bool4_b4
%348 = OpLogicalEqual %v4bool %346 %347
%349 = OpAll %bool %348
OpBranch %345
%345 = OpLabel
%350 = OpPhi %bool %false %338 %349 %344
OpSelectionMerge %352 None
OpBranchConditional %350 %351 %352
%351 = OpLabel
%353 = OpLoad %int %x19
%354 = OpFunctionCall %int %returns_int_i
%355 = OpIEqual %bool %353 %354
OpBranch %352
%352 = OpLabel
%356 = OpPhi %bool %false %345 %355 %351
OpSelectionMerge %358 None
OpBranchConditional %356 %357 %358
%357 = OpLabel
%359 = OpLoad %v2int %x20
%360 = OpFunctionCall %v2int %returns_int2_i2
%361 = OpIEqual %v2bool %359 %360
%362 = OpAll %bool %361
OpBranch %358
%358 = OpLabel
%363 = OpPhi %bool %false %352 %362 %357
OpSelectionMerge %365 None
OpBranchConditional %363 %364 %365
%364 = OpLabel
%366 = OpLoad %v3int %x21
%367 = OpFunctionCall %v3int %returns_int3_i3
%368 = OpIEqual %v3bool %366 %367
%369 = OpAll %bool %368
OpBranch %365
%365 = OpLabel
%370 = OpPhi %bool %false %358 %369 %364
OpSelectionMerge %372 None
OpBranchConditional %370 %371 %372
%371 = OpLabel
%373 = OpLoad %v4int %x22
%374 = OpFunctionCall %v4int %returns_int4_i4
%375 = OpIEqual %v4bool %373 %374
%376 = OpAll %bool %375
OpBranch %372
%372 = OpLabel
%377 = OpPhi %bool %false %365 %376 %371
OpSelectionMerge %381 None
OpBranchConditional %377 %379 %380
%379 = OpLabel
%382 = OpAccessChain %_ptr_Uniform_v4float %31 %int_0
%385 = OpLoad %v4float %382
OpStore %378 %385
OpBranch %381
%380 = OpLabel
%386 = OpAccessChain %_ptr_Uniform_v4float %31 %int_1
%387 = OpLoad %v4float %386
OpStore %378 %387
OpBranch %381
%381 = OpLabel
%388 = OpLoad %v4float %378
OpReturnValue %388
OpFunctionEnd
