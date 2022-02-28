### Compilation failed:

error: SPIR-V validation error: Expected floating scalar or vector type as Result Type: FNegate
  %173 = OpFNegate %mat3v3float %174

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
OpName %test_ivec_b "test_ivec_b"
OpName %one "one"
OpName %two "two"
OpName %one_splat "one_splat"
OpName %one_compound "one_compound"
OpName %ok "ok"
OpName %test_mat_b "test_mat_b"
OpName %one_0 "one"
OpName %one_diagonal "one_diagonal"
OpName %one_compound_0 "one_compound"
OpName %ok_0 "ok"
OpName %main "main"
OpName %_0_one "_0_one"
OpName %_1_two "_1_two"
OpName %_2_one_splat "_2_one_splat"
OpName %_3_one_compound "_3_one_compound"
OpName %_4_ok "_4_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %_2_one_splat RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %_3_one_compound RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %434 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %465 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %475 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
OpDecorate %487 RelaxedPrecision
OpDecorate %491 RelaxedPrecision
OpDecorate %505 RelaxedPrecision
OpDecorate %508 RelaxedPrecision
OpDecorate %509 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%int_n1 = OpConstant %int -1
%57 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n1 %int_n1
%v4bool = OpTypeVector %bool 4
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%_ptr_Function_float = OpTypePointer Function %float
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n1 = OpConstant %float -1
%v3bool = OpTypeVector %bool 3
%float_n0 = OpConstant %float -0
%361 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%384 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %25
%26 = OpLabel
%one = OpVariable %_ptr_Function_int Function
%two = OpVariable %_ptr_Function_int Function
%one_splat = OpVariable %_ptr_Function_v4int Function
%one_compound = OpVariable %_ptr_Function_v4int Function
%ok = OpVariable %_ptr_Function_bool Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%33 = OpLoad %v4float %30
%34 = OpCompositeExtract %float %33 1
%35 = OpConvertFToS %int %34
OpStore %one %35
OpStore %two %int_2
%41 = OpLoad %int %one
%42 = OpCompositeConstruct %v4int %41 %41 %41 %41
OpStore %one_splat %42
%44 = OpLoad %int %one
%45 = OpLoad %int %one
%46 = OpLoad %int %one
%47 = OpLoad %int %one
%48 = OpCompositeConstruct %v4int %44 %45 %46 %47
OpStore %one_compound %48
OpStore %ok %true
%53 = OpLoad %bool %ok
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%59 = OpLoad %v4int %one_splat
%58 = OpSNegate %v4int %59
%60 = OpIEqual %v4bool %57 %58
%62 = OpAll %bool %60
OpBranch %55
%55 = OpLabel
%63 = OpPhi %bool %false %26 %62 %54
OpStore %ok %63
%64 = OpLoad %bool %ok
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpLoad %v4int %one_splat
%67 = OpSNegate %v4int %68
%69 = OpIEqual %v4bool %57 %67
%70 = OpAll %bool %69
OpBranch %66
%66 = OpLabel
%71 = OpPhi %bool %false %55 %70 %65
OpStore %ok %71
%72 = OpLoad %bool %ok
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpLoad %v4int %one_compound
%75 = OpSNegate %v4int %76
%77 = OpIEqual %v4bool %57 %75
%78 = OpAll %bool %77
OpBranch %74
%74 = OpLabel
%79 = OpPhi %bool %false %66 %78 %73
OpStore %ok %79
%80 = OpLoad %bool %ok
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpLoad %v4int %one_compound
%83 = OpSNegate %v4int %84
%85 = OpIEqual %v4bool %57 %83
%86 = OpAll %bool %85
OpBranch %82
%82 = OpLabel
%87 = OpPhi %bool %false %74 %86 %81
OpStore %ok %87
%88 = OpLoad %bool %ok
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpLoad %v4int %one_splat
%91 = OpSNegate %v4int %92
%93 = OpIEqual %v4bool %57 %91
%94 = OpAll %bool %93
OpBranch %90
%90 = OpLabel
%95 = OpPhi %bool %false %82 %94 %89
OpStore %ok %95
%96 = OpLoad %bool %ok
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpLoad %v4int %one_splat
%99 = OpSNegate %v4int %100
%101 = OpIEqual %v4bool %57 %99
%102 = OpAll %bool %101
OpBranch %98
%98 = OpLabel
%103 = OpPhi %bool %false %90 %102 %97
OpStore %ok %103
%104 = OpLoad %bool %ok
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpLoad %v4int %one_compound
%107 = OpSNegate %v4int %108
%109 = OpIEqual %v4bool %57 %107
%110 = OpAll %bool %109
OpBranch %106
%106 = OpLabel
%111 = OpPhi %bool %false %98 %110 %105
OpStore %ok %111
%112 = OpLoad %bool %ok
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpLoad %v4int %one_compound
%115 = OpSNegate %v4int %116
%117 = OpIEqual %v4bool %57 %115
%118 = OpAll %bool %117
OpBranch %114
%114 = OpLabel
%119 = OpPhi %bool %false %106 %118 %113
OpStore %ok %119
%120 = OpLoad %bool %ok
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%126 = OpLoad %int %one
%125 = OpSNegate %int %126
%127 = OpLoad %int %one
%128 = OpLoad %int %one
%129 = OpIAdd %int %127 %128
%130 = OpCompositeConstruct %v2int %125 %129
%123 = OpSNegate %v2int %130
%132 = OpLoad %int %one
%133 = OpLoad %int %two
%134 = OpISub %int %132 %133
%135 = OpCompositeConstruct %v2int %134 %int_2
%131 = OpSNegate %v2int %135
%136 = OpIEqual %v2bool %123 %131
%138 = OpAll %bool %136
OpBranch %122
%122 = OpLabel
%139 = OpPhi %bool %false %114 %138 %121
OpStore %ok %139
%140 = OpLoad %bool %ok
OpReturnValue %140
OpFunctionEnd
%test_mat_b = OpFunction %bool None %25
%141 = OpLabel
%one_0 = OpVariable %_ptr_Function_float Function
%one_diagonal = OpVariable %_ptr_Function_mat3v3float Function
%one_compound_0 = OpVariable %_ptr_Function_mat3v3float Function
%ok_0 = OpVariable %_ptr_Function_bool Function
%144 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%145 = OpLoad %v4float %144
%146 = OpCompositeExtract %float %145 1
OpStore %one_0 %146
%151 = OpLoad %float %one_0
%153 = OpCompositeConstruct %v3float %151 %float_0 %float_0
%154 = OpCompositeConstruct %v3float %float_0 %151 %float_0
%155 = OpCompositeConstruct %v3float %float_0 %float_0 %151
%152 = OpCompositeConstruct %mat3v3float %153 %154 %155
OpStore %one_diagonal %152
%157 = OpLoad %float %one_0
%158 = OpLoad %float %one_0
%159 = OpLoad %float %one_0
%160 = OpCompositeConstruct %v3float %157 %float_0 %float_0
%161 = OpCompositeConstruct %v3float %float_0 %158 %float_0
%162 = OpCompositeConstruct %v3float %float_0 %float_0 %159
%163 = OpCompositeConstruct %mat3v3float %160 %161 %162
OpStore %one_compound_0 %163
OpStore %ok_0 %true
%165 = OpLoad %bool %ok_0
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%170 = OpCompositeConstruct %v3float %float_n1 %float_0 %float_0
%171 = OpCompositeConstruct %v3float %float_0 %float_n1 %float_0
%172 = OpCompositeConstruct %v3float %float_0 %float_0 %float_n1
%169 = OpCompositeConstruct %mat3v3float %170 %171 %172
%174 = OpLoad %mat3v3float %one_diagonal
%173 = OpFNegate %mat3v3float %174
%176 = OpCompositeExtract %v3float %169 0
%177 = OpCompositeExtract %v3float %173 0
%178 = OpFOrdEqual %v3bool %176 %177
%179 = OpAll %bool %178
%180 = OpCompositeExtract %v3float %169 1
%181 = OpCompositeExtract %v3float %173 1
%182 = OpFOrdEqual %v3bool %180 %181
%183 = OpAll %bool %182
%184 = OpLogicalAnd %bool %179 %183
%185 = OpCompositeExtract %v3float %169 2
%186 = OpCompositeExtract %v3float %173 2
%187 = OpFOrdEqual %v3bool %185 %186
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %184 %188
OpBranch %167
%167 = OpLabel
%190 = OpPhi %bool %false %141 %189 %166
OpStore %ok_0 %190
%191 = OpLoad %bool %ok_0
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%194 = OpCompositeConstruct %v3float %float_n1 %float_0 %float_0
%195 = OpCompositeConstruct %v3float %float_0 %float_n1 %float_0
%196 = OpCompositeConstruct %v3float %float_0 %float_0 %float_n1
%197 = OpCompositeConstruct %mat3v3float %194 %195 %196
%199 = OpLoad %mat3v3float %one_diagonal
%198 = OpFNegate %mat3v3float %199
%200 = OpCompositeExtract %v3float %197 0
%201 = OpCompositeExtract %v3float %198 0
%202 = OpFOrdEqual %v3bool %200 %201
%203 = OpAll %bool %202
%204 = OpCompositeExtract %v3float %197 1
%205 = OpCompositeExtract %v3float %198 1
%206 = OpFOrdEqual %v3bool %204 %205
%207 = OpAll %bool %206
%208 = OpLogicalAnd %bool %203 %207
%209 = OpCompositeExtract %v3float %197 2
%210 = OpCompositeExtract %v3float %198 2
%211 = OpFOrdEqual %v3bool %209 %210
%212 = OpAll %bool %211
%213 = OpLogicalAnd %bool %208 %212
OpBranch %193
%193 = OpLabel
%214 = OpPhi %bool %false %167 %213 %192
OpStore %ok_0 %214
%215 = OpLoad %bool %ok_0
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%219 = OpCompositeConstruct %v3float %float_n1 %float_0 %float_0
%220 = OpCompositeConstruct %v3float %float_0 %float_n1 %float_0
%221 = OpCompositeConstruct %v3float %float_0 %float_0 %float_n1
%218 = OpCompositeConstruct %mat3v3float %219 %220 %221
%223 = OpLoad %mat3v3float %one_compound_0
%222 = OpFNegate %mat3v3float %223
%224 = OpCompositeExtract %v3float %218 0
%225 = OpCompositeExtract %v3float %222 0
%226 = OpFOrdEqual %v3bool %224 %225
%227 = OpAll %bool %226
%228 = OpCompositeExtract %v3float %218 1
%229 = OpCompositeExtract %v3float %222 1
%230 = OpFOrdEqual %v3bool %228 %229
%231 = OpAll %bool %230
%232 = OpLogicalAnd %bool %227 %231
%233 = OpCompositeExtract %v3float %218 2
%234 = OpCompositeExtract %v3float %222 2
%235 = OpFOrdEqual %v3bool %233 %234
%236 = OpAll %bool %235
%237 = OpLogicalAnd %bool %232 %236
OpBranch %217
%217 = OpLabel
%238 = OpPhi %bool %false %193 %237 %216
OpStore %ok_0 %238
%239 = OpLoad %bool %ok_0
OpSelectionMerge %241 None
OpBranchConditional %239 %240 %241
%240 = OpLabel
%242 = OpCompositeConstruct %v3float %float_n1 %float_0 %float_0
%243 = OpCompositeConstruct %v3float %float_0 %float_n1 %float_0
%244 = OpCompositeConstruct %v3float %float_0 %float_0 %float_n1
%245 = OpCompositeConstruct %mat3v3float %242 %243 %244
%247 = OpLoad %mat3v3float %one_compound_0
%246 = OpFNegate %mat3v3float %247
%248 = OpCompositeExtract %v3float %245 0
%249 = OpCompositeExtract %v3float %246 0
%250 = OpFOrdEqual %v3bool %248 %249
%251 = OpAll %bool %250
%252 = OpCompositeExtract %v3float %245 1
%253 = OpCompositeExtract %v3float %246 1
%254 = OpFOrdEqual %v3bool %252 %253
%255 = OpAll %bool %254
%256 = OpLogicalAnd %bool %251 %255
%257 = OpCompositeExtract %v3float %245 2
%258 = OpCompositeExtract %v3float %246 2
%259 = OpFOrdEqual %v3bool %257 %258
%260 = OpAll %bool %259
%261 = OpLogicalAnd %bool %256 %260
OpBranch %241
%241 = OpLabel
%262 = OpPhi %bool %false %217 %261 %240
OpStore %ok_0 %262
%263 = OpLoad %bool %ok_0
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%267 = OpCompositeConstruct %v3float %float_n1 %float_0 %float_0
%268 = OpCompositeConstruct %v3float %float_0 %float_n1 %float_0
%269 = OpCompositeConstruct %v3float %float_0 %float_0 %float_n1
%266 = OpCompositeConstruct %mat3v3float %267 %268 %269
%271 = OpLoad %mat3v3float %one_diagonal
%270 = OpFNegate %mat3v3float %271
%272 = OpCompositeExtract %v3float %266 0
%273 = OpCompositeExtract %v3float %270 0
%274 = OpFOrdEqual %v3bool %272 %273
%275 = OpAll %bool %274
%276 = OpCompositeExtract %v3float %266 1
%277 = OpCompositeExtract %v3float %270 1
%278 = OpFOrdEqual %v3bool %276 %277
%279 = OpAll %bool %278
%280 = OpLogicalAnd %bool %275 %279
%281 = OpCompositeExtract %v3float %266 2
%282 = OpCompositeExtract %v3float %270 2
%283 = OpFOrdEqual %v3bool %281 %282
%284 = OpAll %bool %283
%285 = OpLogicalAnd %bool %280 %284
OpBranch %265
%265 = OpLabel
%286 = OpPhi %bool %false %241 %285 %264
OpStore %ok_0 %286
%287 = OpLoad %bool %ok_0
OpSelectionMerge %289 None
OpBranchConditional %287 %288 %289
%288 = OpLabel
%291 = OpCompositeConstruct %v3float %float_n1 %float_n0 %float_n0
%292 = OpCompositeConstruct %v3float %float_n0 %float_n1 %float_n0
%293 = OpCompositeConstruct %v3float %float_n0 %float_n0 %float_n1
%294 = OpCompositeConstruct %mat3v3float %291 %292 %293
%296 = OpLoad %mat3v3float %one_diagonal
%295 = OpFNegate %mat3v3float %296
%297 = OpCompositeExtract %v3float %294 0
%298 = OpCompositeExtract %v3float %295 0
%299 = OpFOrdEqual %v3bool %297 %298
%300 = OpAll %bool %299
%301 = OpCompositeExtract %v3float %294 1
%302 = OpCompositeExtract %v3float %295 1
%303 = OpFOrdEqual %v3bool %301 %302
%304 = OpAll %bool %303
%305 = OpLogicalAnd %bool %300 %304
%306 = OpCompositeExtract %v3float %294 2
%307 = OpCompositeExtract %v3float %295 2
%308 = OpFOrdEqual %v3bool %306 %307
%309 = OpAll %bool %308
%310 = OpLogicalAnd %bool %305 %309
OpBranch %289
%289 = OpLabel
%311 = OpPhi %bool %false %265 %310 %288
OpStore %ok_0 %311
%312 = OpLoad %bool %ok_0
OpSelectionMerge %314 None
OpBranchConditional %312 %313 %314
%313 = OpLabel
%316 = OpCompositeConstruct %v3float %float_n1 %float_0 %float_0
%317 = OpCompositeConstruct %v3float %float_0 %float_n1 %float_0
%318 = OpCompositeConstruct %v3float %float_0 %float_0 %float_n1
%315 = OpCompositeConstruct %mat3v3float %316 %317 %318
%320 = OpLoad %mat3v3float %one_compound_0
%319 = OpFNegate %mat3v3float %320
%321 = OpCompositeExtract %v3float %315 0
%322 = OpCompositeExtract %v3float %319 0
%323 = OpFOrdEqual %v3bool %321 %322
%324 = OpAll %bool %323
%325 = OpCompositeExtract %v3float %315 1
%326 = OpCompositeExtract %v3float %319 1
%327 = OpFOrdEqual %v3bool %325 %326
%328 = OpAll %bool %327
%329 = OpLogicalAnd %bool %324 %328
%330 = OpCompositeExtract %v3float %315 2
%331 = OpCompositeExtract %v3float %319 2
%332 = OpFOrdEqual %v3bool %330 %331
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %329 %333
OpBranch %314
%314 = OpLabel
%335 = OpPhi %bool %false %289 %334 %313
OpStore %ok_0 %335
%336 = OpLoad %bool %ok_0
OpSelectionMerge %338 None
OpBranchConditional %336 %337 %338
%337 = OpLabel
%339 = OpCompositeConstruct %v3float %float_n1 %float_n0 %float_n0
%340 = OpCompositeConstruct %v3float %float_n0 %float_n1 %float_n0
%341 = OpCompositeConstruct %v3float %float_n0 %float_n0 %float_n1
%342 = OpCompositeConstruct %mat3v3float %339 %340 %341
%344 = OpLoad %mat3v3float %one_compound_0
%343 = OpFNegate %mat3v3float %344
%345 = OpCompositeExtract %v3float %342 0
%346 = OpCompositeExtract %v3float %343 0
%347 = OpFOrdEqual %v3bool %345 %346
%348 = OpAll %bool %347
%349 = OpCompositeExtract %v3float %342 1
%350 = OpCompositeExtract %v3float %343 1
%351 = OpFOrdEqual %v3bool %349 %350
%352 = OpAll %bool %351
%353 = OpLogicalAnd %bool %348 %352
%354 = OpCompositeExtract %v3float %342 2
%355 = OpCompositeExtract %v3float %343 2
%356 = OpFOrdEqual %v3bool %354 %355
%357 = OpAll %bool %356
%358 = OpLogicalAnd %bool %353 %357
OpBranch %338
%338 = OpLabel
%359 = OpPhi %bool %false %314 %358 %337
OpStore %ok_0 %359
%360 = OpLoad %bool %ok_0
OpReturnValue %360
OpFunctionEnd
%main = OpFunction %v4float None %361
%362 = OpFunctionParameter %_ptr_Function_v2float
%363 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_2_one_splat = OpVariable %_ptr_Function_v4float Function
%_3_one_compound = OpVariable %_ptr_Function_v4float Function
%_4_ok = OpVariable %_ptr_Function_bool Function
%500 = OpVariable %_ptr_Function_v4float Function
%365 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%366 = OpLoad %v4float %365
%367 = OpCompositeExtract %float %366 1
OpStore %_0_one %367
OpStore %_1_two %float_2
%372 = OpLoad %float %_0_one
%373 = OpCompositeConstruct %v4float %372 %372 %372 %372
OpStore %_2_one_splat %373
%375 = OpLoad %float %_0_one
%376 = OpLoad %float %_0_one
%377 = OpLoad %float %_0_one
%378 = OpLoad %float %_0_one
%379 = OpCompositeConstruct %v4float %375 %376 %377 %378
OpStore %_3_one_compound %379
OpStore %_4_ok %true
%381 = OpLoad %bool %_4_ok
OpSelectionMerge %383 None
OpBranchConditional %381 %382 %383
%382 = OpLabel
%386 = OpLoad %v4float %_2_one_splat
%385 = OpFNegate %v4float %386
%387 = OpFOrdEqual %v4bool %384 %385
%388 = OpAll %bool %387
OpBranch %383
%383 = OpLabel
%389 = OpPhi %bool %false %363 %388 %382
OpStore %_4_ok %389
%390 = OpLoad %bool %_4_ok
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%394 = OpLoad %v4float %_2_one_splat
%393 = OpFNegate %v4float %394
%395 = OpFOrdEqual %v4bool %384 %393
%396 = OpAll %bool %395
OpBranch %392
%392 = OpLabel
%397 = OpPhi %bool %false %383 %396 %391
OpStore %_4_ok %397
%398 = OpLoad %bool %_4_ok
OpSelectionMerge %400 None
OpBranchConditional %398 %399 %400
%399 = OpLabel
%402 = OpLoad %v4float %_3_one_compound
%401 = OpFNegate %v4float %402
%403 = OpFOrdEqual %v4bool %384 %401
%404 = OpAll %bool %403
OpBranch %400
%400 = OpLabel
%405 = OpPhi %bool %false %392 %404 %399
OpStore %_4_ok %405
%406 = OpLoad %bool %_4_ok
OpSelectionMerge %408 None
OpBranchConditional %406 %407 %408
%407 = OpLabel
%410 = OpLoad %v4float %_3_one_compound
%409 = OpFNegate %v4float %410
%411 = OpFOrdEqual %v4bool %384 %409
%412 = OpAll %bool %411
OpBranch %408
%408 = OpLabel
%413 = OpPhi %bool %false %400 %412 %407
OpStore %_4_ok %413
%414 = OpLoad %bool %_4_ok
OpSelectionMerge %416 None
OpBranchConditional %414 %415 %416
%415 = OpLabel
%418 = OpLoad %v4float %_2_one_splat
%417 = OpFNegate %v4float %418
%419 = OpFOrdEqual %v4bool %384 %417
%420 = OpAll %bool %419
OpBranch %416
%416 = OpLabel
%421 = OpPhi %bool %false %408 %420 %415
OpStore %_4_ok %421
%422 = OpLoad %bool %_4_ok
OpSelectionMerge %424 None
OpBranchConditional %422 %423 %424
%423 = OpLabel
%426 = OpLoad %v4float %_2_one_splat
%425 = OpFNegate %v4float %426
%427 = OpFOrdEqual %v4bool %384 %425
%428 = OpAll %bool %427
OpBranch %424
%424 = OpLabel
%429 = OpPhi %bool %false %416 %428 %423
OpStore %_4_ok %429
%430 = OpLoad %bool %_4_ok
OpSelectionMerge %432 None
OpBranchConditional %430 %431 %432
%431 = OpLabel
%434 = OpLoad %v4float %_3_one_compound
%433 = OpFNegate %v4float %434
%435 = OpFOrdEqual %v4bool %384 %433
%436 = OpAll %bool %435
OpBranch %432
%432 = OpLabel
%437 = OpPhi %bool %false %424 %436 %431
OpStore %_4_ok %437
%438 = OpLoad %bool %_4_ok
OpSelectionMerge %440 None
OpBranchConditional %438 %439 %440
%439 = OpLabel
%442 = OpLoad %v4float %_3_one_compound
%441 = OpFNegate %v4float %442
%443 = OpFOrdEqual %v4bool %384 %441
%444 = OpAll %bool %443
OpBranch %440
%440 = OpLabel
%445 = OpPhi %bool %false %432 %444 %439
OpStore %_4_ok %445
%446 = OpLoad %bool %_4_ok
OpSelectionMerge %448 None
OpBranchConditional %446 %447 %448
%447 = OpLabel
%450 = OpLoad %v4float %_3_one_compound
%449 = OpFNegate %v4float %450
%451 = OpFOrdEqual %v4bool %384 %449
%452 = OpAll %bool %451
OpBranch %448
%448 = OpLabel
%453 = OpPhi %bool %false %440 %452 %447
OpStore %_4_ok %453
%454 = OpLoad %bool %_4_ok
OpSelectionMerge %456 None
OpBranchConditional %454 %455 %456
%455 = OpLabel
%458 = OpLoad %float %_1_two
%459 = OpCompositeConstruct %v4float %458 %458 %458 %458
%457 = OpFNegate %v4float %459
%461 = OpLoad %float %_1_two
%460 = OpFNegate %float %461
%463 = OpLoad %float %_1_two
%462 = OpFNegate %float %463
%464 = OpCompositeConstruct %v3float %462 %462 %462
%465 = OpCompositeExtract %float %464 0
%466 = OpCompositeExtract %float %464 1
%467 = OpCompositeExtract %float %464 2
%468 = OpCompositeConstruct %v4float %460 %465 %466 %467
%469 = OpFOrdEqual %v4bool %457 %468
%470 = OpAll %bool %469
OpBranch %456
%456 = OpLabel
%471 = OpPhi %bool %false %448 %470 %455
OpStore %_4_ok %471
%472 = OpLoad %bool %_4_ok
OpSelectionMerge %474 None
OpBranchConditional %472 %473 %474
%473 = OpLabel
%477 = OpLoad %float %_0_one
%476 = OpFNegate %float %477
%478 = OpLoad %float %_0_one
%479 = OpLoad %float %_0_one
%480 = OpFAdd %float %478 %479
%481 = OpCompositeConstruct %v2float %476 %480
%475 = OpFNegate %v2float %481
%483 = OpLoad %float %_0_one
%484 = OpLoad %float %_1_two
%485 = OpFSub %float %483 %484
%486 = OpLoad %float %_1_two
%487 = OpCompositeConstruct %v2float %485 %486
%482 = OpFNegate %v2float %487
%488 = OpFOrdEqual %v2bool %475 %482
%489 = OpAll %bool %488
OpBranch %474
%474 = OpLabel
%490 = OpPhi %bool %false %456 %489 %473
OpStore %_4_ok %490
%491 = OpLoad %bool %_4_ok
OpSelectionMerge %493 None
OpBranchConditional %491 %492 %493
%492 = OpLabel
%494 = OpFunctionCall %bool %test_ivec_b
OpBranch %493
%493 = OpLabel
%495 = OpPhi %bool %false %474 %494 %492
OpSelectionMerge %497 None
OpBranchConditional %495 %496 %497
%496 = OpLabel
%498 = OpFunctionCall %bool %test_mat_b
OpBranch %497
%497 = OpLabel
%499 = OpPhi %bool %false %493 %498 %496
OpSelectionMerge %503 None
OpBranchConditional %499 %501 %502
%501 = OpLabel
%504 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%505 = OpLoad %v4float %504
OpStore %500 %505
OpBranch %503
%502 = OpLabel
%506 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%508 = OpLoad %v4float %506
OpStore %500 %508
OpBranch %503
%503 = OpLabel
%509 = OpLoad %v4float %500
OpReturnValue %509
OpFunctionEnd

1 error
