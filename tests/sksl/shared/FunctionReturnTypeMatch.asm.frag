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
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %33 Binding 0
OpDecorate %33 DescriptorSet 0
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %x8 RelaxedPrecision
OpDecorate %x9 RelaxedPrecision
OpDecorate %x10 RelaxedPrecision
OpDecorate %x11 RelaxedPrecision
OpDecorate %x12 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %x13 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %x14 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%33 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%38 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%42 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%46 = OpTypeFunction %v2float
%float_2 = OpConstant %float 2
%49 = OpConstantComposite %v2float %float_2 %float_2
%v3float = OpTypeVector %float 3
%51 = OpTypeFunction %v3float
%float_3 = OpConstant %float 3
%54 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%55 = OpTypeFunction %v4float
%float_4 = OpConstant %float 4
%58 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%mat2v2float = OpTypeMatrix %v2float 2
%60 = OpTypeFunction %mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%66 = OpTypeFunction %mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%73 = OpTypeFunction %mat4v4float
%80 = OpTypeFunction %float
%float_1 = OpConstant %float 1
%101 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%105 = OpTypeFunction %v2bool
%107 = OpConstantComposite %v2bool %true %true
%v3bool = OpTypeVector %bool 3
%109 = OpTypeFunction %v3bool
%111 = OpConstantComposite %v3bool %true %true %true
%v4bool = OpTypeVector %bool 4
%113 = OpTypeFunction %v4bool
%115 = OpConstantComposite %v4bool %true %true %true %true
%int = OpTypeInt 32 1
%117 = OpTypeFunction %int
%int_1 = OpConstant %int 1
%v2int = OpTypeVector %int 2
%121 = OpTypeFunction %v2int
%int_2 = OpConstant %int 2
%124 = OpConstantComposite %v2int %int_2 %int_2
%v3int = OpTypeVector %int 3
%126 = OpTypeFunction %v3int
%int_3 = OpConstant %int 3
%129 = OpConstantComposite %v3int %int_3 %int_3 %int_3
%v4int = OpTypeVector %int 4
%131 = OpTypeFunction %v4int
%int_4 = OpConstant %int 4
%134 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%135 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%_entrypoint_v = OpFunction %void None %38
%39 = OpLabel
%43 = OpVariable %_ptr_Function_v2float Function
OpStore %43 %42
%45 = OpFunctionCall %v4float %main %43
OpStore %sk_FragColor %45
OpReturn
OpFunctionEnd
%returns_float2_f2 = OpFunction %v2float None %46
%47 = OpLabel
OpReturnValue %49
OpFunctionEnd
%returns_float3_f3 = OpFunction %v3float None %51
%52 = OpLabel
OpReturnValue %54
OpFunctionEnd
%returns_float4_f4 = OpFunction %v4float None %55
%56 = OpLabel
OpReturnValue %58
OpFunctionEnd
%returns_float2x2_f22 = OpFunction %mat2v2float None %60
%61 = OpLabel
%63 = OpCompositeConstruct %v2float %float_2 %float_0
%64 = OpCompositeConstruct %v2float %float_0 %float_2
%62 = OpCompositeConstruct %mat2v2float %63 %64
OpReturnValue %62
OpFunctionEnd
%returns_float3x3_f33 = OpFunction %mat3v3float None %66
%67 = OpLabel
%69 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%70 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%71 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%68 = OpCompositeConstruct %mat3v3float %69 %70 %71
OpReturnValue %68
OpFunctionEnd
%returns_float4x4_f44 = OpFunction %mat4v4float None %73
%74 = OpLabel
%76 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%77 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%78 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%79 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%75 = OpCompositeConstruct %mat4v4float %76 %77 %78 %79
OpReturnValue %75
OpFunctionEnd
%returns_half_h = OpFunction %float None %80
%81 = OpLabel
OpReturnValue %float_1
OpFunctionEnd
%returns_half2_h2 = OpFunction %v2float None %46
%83 = OpLabel
OpReturnValue %49
OpFunctionEnd
%returns_half3_h3 = OpFunction %v3float None %51
%84 = OpLabel
OpReturnValue %54
OpFunctionEnd
%returns_half4_h4 = OpFunction %v4float None %55
%85 = OpLabel
OpReturnValue %58
OpFunctionEnd
%returns_half2x2_h22 = OpFunction %mat2v2float None %60
%86 = OpLabel
%88 = OpCompositeConstruct %v2float %float_2 %float_0
%89 = OpCompositeConstruct %v2float %float_0 %float_2
%87 = OpCompositeConstruct %mat2v2float %88 %89
OpReturnValue %87
OpFunctionEnd
%returns_half3x3_h33 = OpFunction %mat3v3float None %66
%90 = OpLabel
%92 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%93 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%94 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%91 = OpCompositeConstruct %mat3v3float %92 %93 %94
OpReturnValue %91
OpFunctionEnd
%returns_half4x4_h44 = OpFunction %mat4v4float None %73
%95 = OpLabel
%97 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%98 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%99 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%100 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%96 = OpCompositeConstruct %mat4v4float %97 %98 %99 %100
OpReturnValue %96
OpFunctionEnd
%returns_bool_b = OpFunction %bool None %101
%102 = OpLabel
OpReturnValue %true
OpFunctionEnd
%returns_bool2_b2 = OpFunction %v2bool None %105
%106 = OpLabel
OpReturnValue %107
OpFunctionEnd
%returns_bool3_b3 = OpFunction %v3bool None %109
%110 = OpLabel
OpReturnValue %111
OpFunctionEnd
%returns_bool4_b4 = OpFunction %v4bool None %113
%114 = OpLabel
OpReturnValue %115
OpFunctionEnd
%returns_int_i = OpFunction %int None %117
%118 = OpLabel
OpReturnValue %int_1
OpFunctionEnd
%returns_int2_i2 = OpFunction %v2int None %121
%122 = OpLabel
OpReturnValue %124
OpFunctionEnd
%returns_int3_i3 = OpFunction %v3int None %126
%127 = OpLabel
OpReturnValue %129
OpFunctionEnd
%returns_int4_i4 = OpFunction %v4int None %131
%132 = OpLabel
OpReturnValue %134
OpFunctionEnd
%main = OpFunction %v4float None %135
%136 = OpFunctionParameter %_ptr_Function_v2float
%137 = OpLabel
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
%417 = OpVariable %_ptr_Function_v4float Function
OpStore %x1 %float_1
OpStore %x2 %49
OpStore %x3 %54
OpStore %x4 %58
%148 = OpCompositeConstruct %v2float %float_2 %float_0
%149 = OpCompositeConstruct %v2float %float_0 %float_2
%147 = OpCompositeConstruct %mat2v2float %148 %149
OpStore %x5 %147
%153 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%154 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%152 = OpCompositeConstruct %mat3v3float %153 %154 %155
OpStore %x6 %152
%159 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%160 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%161 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%162 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%158 = OpCompositeConstruct %mat4v4float %159 %160 %161 %162
OpStore %x7 %158
OpStore %x8 %float_1
OpStore %x9 %49
OpStore %x10 %54
OpStore %x11 %58
%169 = OpCompositeConstruct %v2float %float_2 %float_0
%170 = OpCompositeConstruct %v2float %float_0 %float_2
%168 = OpCompositeConstruct %mat2v2float %169 %170
OpStore %x12 %168
%173 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%174 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%175 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%172 = OpCompositeConstruct %mat3v3float %173 %174 %175
OpStore %x13 %172
%178 = OpCompositeConstruct %v4float %float_4 %float_0 %float_0 %float_0
%179 = OpCompositeConstruct %v4float %float_0 %float_4 %float_0 %float_0
%180 = OpCompositeConstruct %v4float %float_0 %float_0 %float_4 %float_0
%181 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_4
%177 = OpCompositeConstruct %mat4v4float %178 %179 %180 %181
OpStore %x14 %177
OpStore %x15 %true
OpStore %x16 %107
OpStore %x17 %111
OpStore %x18 %115
OpStore %x19 %int_1
OpStore %x20 %124
OpStore %x21 %129
OpStore %x22 %134
%199 = OpLoad %float %x1
%200 = OpFOrdEqual %bool %199 %float_1
OpSelectionMerge %202 None
OpBranchConditional %200 %201 %202
%201 = OpLabel
%203 = OpLoad %v2float %x2
%204 = OpFunctionCall %v2float %returns_float2_f2
%205 = OpFOrdEqual %v2bool %203 %204
%206 = OpAll %bool %205
OpBranch %202
%202 = OpLabel
%207 = OpPhi %bool %false %137 %206 %201
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
%210 = OpLoad %v3float %x3
%211 = OpFunctionCall %v3float %returns_float3_f3
%212 = OpFOrdEqual %v3bool %210 %211
%213 = OpAll %bool %212
OpBranch %209
%209 = OpLabel
%214 = OpPhi %bool %false %202 %213 %208
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%217 = OpLoad %v4float %x4
%218 = OpFunctionCall %v4float %returns_float4_f4
%219 = OpFOrdEqual %v4bool %217 %218
%220 = OpAll %bool %219
OpBranch %216
%216 = OpLabel
%221 = OpPhi %bool %false %209 %220 %215
OpSelectionMerge %223 None
OpBranchConditional %221 %222 %223
%222 = OpLabel
%224 = OpLoad %mat2v2float %x5
%225 = OpFunctionCall %mat2v2float %returns_float2x2_f22
%226 = OpCompositeExtract %v2float %224 0
%227 = OpCompositeExtract %v2float %225 0
%228 = OpFOrdEqual %v2bool %226 %227
%229 = OpAll %bool %228
%230 = OpCompositeExtract %v2float %224 1
%231 = OpCompositeExtract %v2float %225 1
%232 = OpFOrdEqual %v2bool %230 %231
%233 = OpAll %bool %232
%234 = OpLogicalAnd %bool %229 %233
OpBranch %223
%223 = OpLabel
%235 = OpPhi %bool %false %216 %234 %222
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%238 = OpLoad %mat3v3float %x6
%239 = OpFunctionCall %mat3v3float %returns_float3x3_f33
%240 = OpCompositeExtract %v3float %238 0
%241 = OpCompositeExtract %v3float %239 0
%242 = OpFOrdEqual %v3bool %240 %241
%243 = OpAll %bool %242
%244 = OpCompositeExtract %v3float %238 1
%245 = OpCompositeExtract %v3float %239 1
%246 = OpFOrdEqual %v3bool %244 %245
%247 = OpAll %bool %246
%248 = OpLogicalAnd %bool %243 %247
%249 = OpCompositeExtract %v3float %238 2
%250 = OpCompositeExtract %v3float %239 2
%251 = OpFOrdEqual %v3bool %249 %250
%252 = OpAll %bool %251
%253 = OpLogicalAnd %bool %248 %252
OpBranch %237
%237 = OpLabel
%254 = OpPhi %bool %false %223 %253 %236
OpSelectionMerge %256 None
OpBranchConditional %254 %255 %256
%255 = OpLabel
%257 = OpLoad %mat4v4float %x7
%258 = OpFunctionCall %mat4v4float %returns_float4x4_f44
%259 = OpCompositeExtract %v4float %257 0
%260 = OpCompositeExtract %v4float %258 0
%261 = OpFOrdEqual %v4bool %259 %260
%262 = OpAll %bool %261
%263 = OpCompositeExtract %v4float %257 1
%264 = OpCompositeExtract %v4float %258 1
%265 = OpFOrdEqual %v4bool %263 %264
%266 = OpAll %bool %265
%267 = OpLogicalAnd %bool %262 %266
%268 = OpCompositeExtract %v4float %257 2
%269 = OpCompositeExtract %v4float %258 2
%270 = OpFOrdEqual %v4bool %268 %269
%271 = OpAll %bool %270
%272 = OpLogicalAnd %bool %267 %271
%273 = OpCompositeExtract %v4float %257 3
%274 = OpCompositeExtract %v4float %258 3
%275 = OpFOrdEqual %v4bool %273 %274
%276 = OpAll %bool %275
%277 = OpLogicalAnd %bool %272 %276
OpBranch %256
%256 = OpLabel
%278 = OpPhi %bool %false %237 %277 %255
OpSelectionMerge %280 None
OpBranchConditional %278 %279 %280
%279 = OpLabel
%281 = OpLoad %float %x8
%282 = OpFunctionCall %float %returns_half_h
%283 = OpFOrdEqual %bool %281 %282
OpBranch %280
%280 = OpLabel
%284 = OpPhi %bool %false %256 %283 %279
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%287 = OpLoad %v2float %x9
%288 = OpFunctionCall %v2float %returns_half2_h2
%289 = OpFOrdEqual %v2bool %287 %288
%290 = OpAll %bool %289
OpBranch %286
%286 = OpLabel
%291 = OpPhi %bool %false %280 %290 %285
OpSelectionMerge %293 None
OpBranchConditional %291 %292 %293
%292 = OpLabel
%294 = OpLoad %v3float %x10
%295 = OpFunctionCall %v3float %returns_half3_h3
%296 = OpFOrdEqual %v3bool %294 %295
%297 = OpAll %bool %296
OpBranch %293
%293 = OpLabel
%298 = OpPhi %bool %false %286 %297 %292
OpSelectionMerge %300 None
OpBranchConditional %298 %299 %300
%299 = OpLabel
%301 = OpLoad %v4float %x11
%302 = OpFunctionCall %v4float %returns_half4_h4
%303 = OpFOrdEqual %v4bool %301 %302
%304 = OpAll %bool %303
OpBranch %300
%300 = OpLabel
%305 = OpPhi %bool %false %293 %304 %299
OpSelectionMerge %307 None
OpBranchConditional %305 %306 %307
%306 = OpLabel
%308 = OpLoad %mat2v2float %x12
%309 = OpFunctionCall %mat2v2float %returns_half2x2_h22
%310 = OpCompositeExtract %v2float %308 0
%311 = OpCompositeExtract %v2float %309 0
%312 = OpFOrdEqual %v2bool %310 %311
%313 = OpAll %bool %312
%314 = OpCompositeExtract %v2float %308 1
%315 = OpCompositeExtract %v2float %309 1
%316 = OpFOrdEqual %v2bool %314 %315
%317 = OpAll %bool %316
%318 = OpLogicalAnd %bool %313 %317
OpBranch %307
%307 = OpLabel
%319 = OpPhi %bool %false %300 %318 %306
OpSelectionMerge %321 None
OpBranchConditional %319 %320 %321
%320 = OpLabel
%322 = OpLoad %mat3v3float %x13
%323 = OpFunctionCall %mat3v3float %returns_half3x3_h33
%324 = OpCompositeExtract %v3float %322 0
%325 = OpCompositeExtract %v3float %323 0
%326 = OpFOrdEqual %v3bool %324 %325
%327 = OpAll %bool %326
%328 = OpCompositeExtract %v3float %322 1
%329 = OpCompositeExtract %v3float %323 1
%330 = OpFOrdEqual %v3bool %328 %329
%331 = OpAll %bool %330
%332 = OpLogicalAnd %bool %327 %331
%333 = OpCompositeExtract %v3float %322 2
%334 = OpCompositeExtract %v3float %323 2
%335 = OpFOrdEqual %v3bool %333 %334
%336 = OpAll %bool %335
%337 = OpLogicalAnd %bool %332 %336
OpBranch %321
%321 = OpLabel
%338 = OpPhi %bool %false %307 %337 %320
OpSelectionMerge %340 None
OpBranchConditional %338 %339 %340
%339 = OpLabel
%341 = OpLoad %mat4v4float %x14
%342 = OpFunctionCall %mat4v4float %returns_half4x4_h44
%343 = OpCompositeExtract %v4float %341 0
%344 = OpCompositeExtract %v4float %342 0
%345 = OpFOrdEqual %v4bool %343 %344
%346 = OpAll %bool %345
%347 = OpCompositeExtract %v4float %341 1
%348 = OpCompositeExtract %v4float %342 1
%349 = OpFOrdEqual %v4bool %347 %348
%350 = OpAll %bool %349
%351 = OpLogicalAnd %bool %346 %350
%352 = OpCompositeExtract %v4float %341 2
%353 = OpCompositeExtract %v4float %342 2
%354 = OpFOrdEqual %v4bool %352 %353
%355 = OpAll %bool %354
%356 = OpLogicalAnd %bool %351 %355
%357 = OpCompositeExtract %v4float %341 3
%358 = OpCompositeExtract %v4float %342 3
%359 = OpFOrdEqual %v4bool %357 %358
%360 = OpAll %bool %359
%361 = OpLogicalAnd %bool %356 %360
OpBranch %340
%340 = OpLabel
%362 = OpPhi %bool %false %321 %361 %339
OpSelectionMerge %364 None
OpBranchConditional %362 %363 %364
%363 = OpLabel
%365 = OpLoad %bool %x15
%366 = OpFunctionCall %bool %returns_bool_b
%367 = OpLogicalEqual %bool %365 %366
OpBranch %364
%364 = OpLabel
%368 = OpPhi %bool %false %340 %367 %363
OpSelectionMerge %370 None
OpBranchConditional %368 %369 %370
%369 = OpLabel
%371 = OpLoad %v2bool %x16
%372 = OpFunctionCall %v2bool %returns_bool2_b2
%373 = OpLogicalEqual %v2bool %371 %372
%374 = OpAll %bool %373
OpBranch %370
%370 = OpLabel
%375 = OpPhi %bool %false %364 %374 %369
OpSelectionMerge %377 None
OpBranchConditional %375 %376 %377
%376 = OpLabel
%378 = OpLoad %v3bool %x17
%379 = OpFunctionCall %v3bool %returns_bool3_b3
%380 = OpLogicalEqual %v3bool %378 %379
%381 = OpAll %bool %380
OpBranch %377
%377 = OpLabel
%382 = OpPhi %bool %false %370 %381 %376
OpSelectionMerge %384 None
OpBranchConditional %382 %383 %384
%383 = OpLabel
%385 = OpLoad %v4bool %x18
%386 = OpFunctionCall %v4bool %returns_bool4_b4
%387 = OpLogicalEqual %v4bool %385 %386
%388 = OpAll %bool %387
OpBranch %384
%384 = OpLabel
%389 = OpPhi %bool %false %377 %388 %383
OpSelectionMerge %391 None
OpBranchConditional %389 %390 %391
%390 = OpLabel
%392 = OpLoad %int %x19
%393 = OpFunctionCall %int %returns_int_i
%394 = OpIEqual %bool %392 %393
OpBranch %391
%391 = OpLabel
%395 = OpPhi %bool %false %384 %394 %390
OpSelectionMerge %397 None
OpBranchConditional %395 %396 %397
%396 = OpLabel
%398 = OpLoad %v2int %x20
%399 = OpFunctionCall %v2int %returns_int2_i2
%400 = OpIEqual %v2bool %398 %399
%401 = OpAll %bool %400
OpBranch %397
%397 = OpLabel
%402 = OpPhi %bool %false %391 %401 %396
OpSelectionMerge %404 None
OpBranchConditional %402 %403 %404
%403 = OpLabel
%405 = OpLoad %v3int %x21
%406 = OpFunctionCall %v3int %returns_int3_i3
%407 = OpIEqual %v3bool %405 %406
%408 = OpAll %bool %407
OpBranch %404
%404 = OpLabel
%409 = OpPhi %bool %false %397 %408 %403
OpSelectionMerge %411 None
OpBranchConditional %409 %410 %411
%410 = OpLabel
%412 = OpLoad %v4int %x22
%413 = OpFunctionCall %v4int %returns_int4_i4
%414 = OpIEqual %v4bool %412 %413
%415 = OpAll %bool %414
OpBranch %411
%411 = OpLabel
%416 = OpPhi %bool %false %404 %415 %410
OpSelectionMerge %420 None
OpBranchConditional %416 %418 %419
%418 = OpLabel
%421 = OpAccessChain %_ptr_Uniform_v4float %33 %int_0
%424 = OpLoad %v4float %421
OpStore %417 %424
OpBranch %420
%419 = OpLabel
%425 = OpAccessChain %_ptr_Uniform_v4float %33 %int_1
%426 = OpLoad %v4float %425
OpStore %417 %426
OpBranch %420
%420 = OpLabel
%427 = OpLoad %v4float %417
OpReturnValue %427
OpFunctionEnd
