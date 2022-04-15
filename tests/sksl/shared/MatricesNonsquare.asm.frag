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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m23 "m23"
OpName %m24 "m24"
OpName %m32 "m32"
OpName %m34 "m34"
OpName %m42 "m42"
OpName %m43 "m43"
OpName %m22 "m22"
OpName %m33 "m33"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
OpName %_7_m22 "_7_m22"
OpName %_8_m33 "_8_m33"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m23 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %482 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%34 = OpConstantComposite %v3float %float_2 %float_0 %float_0
%35 = OpConstantComposite %v3float %float_0 %float_2 %float_0
%36 = OpConstantComposite %mat2v3float %34 %35
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%55 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%56 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%57 = OpConstantComposite %mat2v4float %55 %56
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%74 = OpConstantComposite %v2float %float_4 %float_0
%75 = OpConstantComposite %v2float %float_0 %float_4
%76 = OpConstantComposite %mat3v2float %74 %75 %20
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%97 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%98 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%99 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%100 = OpConstantComposite %mat3v4float %97 %98 %99
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%120 = OpConstantComposite %v2float %float_6 %float_0
%121 = OpConstantComposite %v2float %float_0 %float_6
%122 = OpConstantComposite %mat4v2float %120 %121 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%146 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%147 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%148 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%149 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%150 = OpConstantComposite %mat4v3float %146 %147 %148 %149
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%180 = OpConstantComposite %v2float %float_8 %float_0
%181 = OpConstantComposite %v2float %float_0 %float_8
%182 = OpConstantComposite %mat2v2float %180 %181
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%201 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%202 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%203 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%204 = OpConstantComposite %mat3v3float %201 %202 %203
%float_1 = OpConstant %float 1
%219 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%220 = OpConstantComposite %mat2v3float %219 %219
%228 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%229 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%230 = OpConstantComposite %mat2v3float %228 %229
%238 = OpConstantComposite %v2float %float_2 %float_2
%239 = OpConstantComposite %mat3v2float %238 %238 %238
%float_n2 = OpConstant %float -2
%250 = OpConstantComposite %v2float %float_2 %float_n2
%251 = OpConstantComposite %v2float %float_n2 %float_2
%252 = OpConstantComposite %v2float %float_n2 %float_n2
%253 = OpConstantComposite %mat3v2float %250 %251 %252
%264 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%265 = OpConstantComposite %mat2v4float %264 %264
%float_0_75 = OpConstant %float 0.75
%274 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%275 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%276 = OpConstantComposite %mat2v4float %274 %275
%283 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m23 = OpVariable %_ptr_Function_mat2v3float Function
%m24 = OpVariable %_ptr_Function_mat2v4float Function
%m32 = OpVariable %_ptr_Function_mat3v2float Function
%m34 = OpVariable %_ptr_Function_mat3v4float Function
%m42 = OpVariable %_ptr_Function_mat4v2float Function
%m43 = OpVariable %_ptr_Function_mat4v3float Function
%m22 = OpVariable %_ptr_Function_mat2v2float Function
%m33 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %ok %true
OpStore %m23 %36
%38 = OpLoad %bool %ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat2v3float %m23
%43 = OpCompositeExtract %v3float %41 0
%44 = OpFOrdEqual %v3bool %43 %34
%45 = OpAll %bool %44
%46 = OpCompositeExtract %v3float %41 1
%47 = OpFOrdEqual %v3bool %46 %35
%48 = OpAll %bool %47
%49 = OpLogicalAnd %bool %45 %48
OpBranch %40
%40 = OpLabel
%50 = OpPhi %bool %false %25 %49 %39
OpStore %ok %50
OpStore %m24 %57
OpSelectionMerge %59 None
OpBranchConditional %50 %58 %59
%58 = OpLabel
%60 = OpLoad %mat2v4float %m24
%62 = OpCompositeExtract %v4float %60 0
%63 = OpFOrdEqual %v4bool %62 %55
%64 = OpAll %bool %63
%65 = OpCompositeExtract %v4float %60 1
%66 = OpFOrdEqual %v4bool %65 %56
%67 = OpAll %bool %66
%68 = OpLogicalAnd %bool %64 %67
OpBranch %59
%59 = OpLabel
%69 = OpPhi %bool %false %40 %68 %58
OpStore %ok %69
OpStore %m32 %76
OpSelectionMerge %78 None
OpBranchConditional %69 %77 %78
%77 = OpLabel
%79 = OpLoad %mat3v2float %m32
%81 = OpCompositeExtract %v2float %79 0
%82 = OpFOrdEqual %v2bool %81 %74
%83 = OpAll %bool %82
%84 = OpCompositeExtract %v2float %79 1
%85 = OpFOrdEqual %v2bool %84 %75
%86 = OpAll %bool %85
%87 = OpLogicalAnd %bool %83 %86
%88 = OpCompositeExtract %v2float %79 2
%89 = OpFOrdEqual %v2bool %88 %20
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %87 %90
OpBranch %78
%78 = OpLabel
%92 = OpPhi %bool %false %59 %91 %77
OpStore %ok %92
OpStore %m34 %100
OpSelectionMerge %102 None
OpBranchConditional %92 %101 %102
%101 = OpLabel
%103 = OpLoad %mat3v4float %m34
%104 = OpCompositeExtract %v4float %103 0
%105 = OpFOrdEqual %v4bool %104 %97
%106 = OpAll %bool %105
%107 = OpCompositeExtract %v4float %103 1
%108 = OpFOrdEqual %v4bool %107 %98
%109 = OpAll %bool %108
%110 = OpLogicalAnd %bool %106 %109
%111 = OpCompositeExtract %v4float %103 2
%112 = OpFOrdEqual %v4bool %111 %99
%113 = OpAll %bool %112
%114 = OpLogicalAnd %bool %110 %113
OpBranch %102
%102 = OpLabel
%115 = OpPhi %bool %false %78 %114 %101
OpStore %ok %115
OpStore %m42 %122
OpSelectionMerge %124 None
OpBranchConditional %115 %123 %124
%123 = OpLabel
%125 = OpLoad %mat4v2float %m42
%126 = OpCompositeExtract %v2float %125 0
%127 = OpFOrdEqual %v2bool %126 %120
%128 = OpAll %bool %127
%129 = OpCompositeExtract %v2float %125 1
%130 = OpFOrdEqual %v2bool %129 %121
%131 = OpAll %bool %130
%132 = OpLogicalAnd %bool %128 %131
%133 = OpCompositeExtract %v2float %125 2
%134 = OpFOrdEqual %v2bool %133 %20
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %132 %135
%137 = OpCompositeExtract %v2float %125 3
%138 = OpFOrdEqual %v2bool %137 %20
%139 = OpAll %bool %138
%140 = OpLogicalAnd %bool %136 %139
OpBranch %124
%124 = OpLabel
%141 = OpPhi %bool %false %102 %140 %123
OpStore %ok %141
OpStore %m43 %150
OpSelectionMerge %152 None
OpBranchConditional %141 %151 %152
%151 = OpLabel
%153 = OpLoad %mat4v3float %m43
%154 = OpCompositeExtract %v3float %153 0
%155 = OpFOrdEqual %v3bool %154 %146
%156 = OpAll %bool %155
%157 = OpCompositeExtract %v3float %153 1
%158 = OpFOrdEqual %v3bool %157 %147
%159 = OpAll %bool %158
%160 = OpLogicalAnd %bool %156 %159
%161 = OpCompositeExtract %v3float %153 2
%162 = OpFOrdEqual %v3bool %161 %148
%163 = OpAll %bool %162
%164 = OpLogicalAnd %bool %160 %163
%165 = OpCompositeExtract %v3float %153 3
%166 = OpFOrdEqual %v3bool %165 %149
%167 = OpAll %bool %166
%168 = OpLogicalAnd %bool %164 %167
OpBranch %152
%152 = OpLabel
%169 = OpPhi %bool %false %124 %168 %151
OpStore %ok %169
%173 = OpLoad %mat3v2float %m32
%174 = OpLoad %mat2v3float %m23
%175 = OpMatrixTimesMatrix %mat2v2float %173 %174
OpStore %m22 %175
OpSelectionMerge %177 None
OpBranchConditional %169 %176 %177
%176 = OpLabel
%178 = OpLoad %mat2v2float %m22
%183 = OpCompositeExtract %v2float %178 0
%184 = OpFOrdEqual %v2bool %183 %180
%185 = OpAll %bool %184
%186 = OpCompositeExtract %v2float %178 1
%187 = OpFOrdEqual %v2bool %186 %181
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %185 %188
OpBranch %177
%177 = OpLabel
%190 = OpPhi %bool %false %152 %189 %176
OpStore %ok %190
%194 = OpLoad %mat4v3float %m43
%195 = OpLoad %mat3v4float %m34
%196 = OpMatrixTimesMatrix %mat3v3float %194 %195
OpStore %m33 %196
OpSelectionMerge %198 None
OpBranchConditional %190 %197 %198
%197 = OpLabel
%199 = OpLoad %mat3v3float %m33
%205 = OpCompositeExtract %v3float %199 0
%206 = OpFOrdEqual %v3bool %205 %201
%207 = OpAll %bool %206
%208 = OpCompositeExtract %v3float %199 1
%209 = OpFOrdEqual %v3bool %208 %202
%210 = OpAll %bool %209
%211 = OpLogicalAnd %bool %207 %210
%212 = OpCompositeExtract %v3float %199 2
%213 = OpFOrdEqual %v3bool %212 %203
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %211 %214
OpBranch %198
%198 = OpLabel
%216 = OpPhi %bool %false %177 %215 %197
OpStore %ok %216
%217 = OpLoad %mat2v3float %m23
%221 = OpCompositeExtract %v3float %217 0
%222 = OpFAdd %v3float %221 %219
%223 = OpCompositeExtract %v3float %217 1
%224 = OpFAdd %v3float %223 %219
%225 = OpCompositeConstruct %mat2v3float %222 %224
OpStore %m23 %225
OpSelectionMerge %227 None
OpBranchConditional %216 %226 %227
%226 = OpLabel
%231 = OpFOrdEqual %v3bool %222 %228
%232 = OpAll %bool %231
%233 = OpFOrdEqual %v3bool %224 %229
%234 = OpAll %bool %233
%235 = OpLogicalAnd %bool %232 %234
OpBranch %227
%227 = OpLabel
%236 = OpPhi %bool %false %198 %235 %226
OpStore %ok %236
%237 = OpLoad %mat3v2float %m32
%240 = OpCompositeExtract %v2float %237 0
%241 = OpFSub %v2float %240 %238
%242 = OpCompositeExtract %v2float %237 1
%243 = OpFSub %v2float %242 %238
%244 = OpCompositeExtract %v2float %237 2
%245 = OpFSub %v2float %244 %238
%246 = OpCompositeConstruct %mat3v2float %241 %243 %245
OpStore %m32 %246
OpSelectionMerge %248 None
OpBranchConditional %236 %247 %248
%247 = OpLabel
%254 = OpFOrdEqual %v2bool %241 %250
%255 = OpAll %bool %254
%256 = OpFOrdEqual %v2bool %243 %251
%257 = OpAll %bool %256
%258 = OpLogicalAnd %bool %255 %257
%259 = OpFOrdEqual %v2bool %245 %252
%260 = OpAll %bool %259
%261 = OpLogicalAnd %bool %258 %260
OpBranch %248
%248 = OpLabel
%262 = OpPhi %bool %false %227 %261 %247
OpStore %ok %262
%263 = OpLoad %mat2v4float %m24
%266 = OpCompositeExtract %v4float %263 0
%267 = OpFDiv %v4float %266 %264
%268 = OpCompositeExtract %v4float %263 1
%269 = OpFDiv %v4float %268 %264
%270 = OpCompositeConstruct %mat2v4float %267 %269
OpStore %m24 %270
OpSelectionMerge %272 None
OpBranchConditional %262 %271 %272
%271 = OpLabel
%277 = OpFOrdEqual %v4bool %267 %274
%278 = OpAll %bool %277
%279 = OpFOrdEqual %v4bool %269 %275
%280 = OpAll %bool %279
%281 = OpLogicalAnd %bool %278 %280
OpBranch %272
%272 = OpLabel
%282 = OpPhi %bool %false %248 %281 %271
OpStore %ok %282
OpReturnValue %282
OpFunctionEnd
%main = OpFunction %v4float None %283
%284 = OpFunctionParameter %_ptr_Function_v2float
%285 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%469 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m23 %36
%288 = OpLoad %bool %_0_ok
OpSelectionMerge %290 None
OpBranchConditional %288 %289 %290
%289 = OpLabel
%291 = OpLoad %mat2v3float %_1_m23
%292 = OpCompositeExtract %v3float %291 0
%293 = OpFOrdEqual %v3bool %292 %34
%294 = OpAll %bool %293
%295 = OpCompositeExtract %v3float %291 1
%296 = OpFOrdEqual %v3bool %295 %35
%297 = OpAll %bool %296
%298 = OpLogicalAnd %bool %294 %297
OpBranch %290
%290 = OpLabel
%299 = OpPhi %bool %false %285 %298 %289
OpStore %_0_ok %299
OpStore %_2_m24 %57
OpSelectionMerge %302 None
OpBranchConditional %299 %301 %302
%301 = OpLabel
%303 = OpLoad %mat2v4float %_2_m24
%304 = OpCompositeExtract %v4float %303 0
%305 = OpFOrdEqual %v4bool %304 %55
%306 = OpAll %bool %305
%307 = OpCompositeExtract %v4float %303 1
%308 = OpFOrdEqual %v4bool %307 %56
%309 = OpAll %bool %308
%310 = OpLogicalAnd %bool %306 %309
OpBranch %302
%302 = OpLabel
%311 = OpPhi %bool %false %290 %310 %301
OpStore %_0_ok %311
OpStore %_3_m32 %76
OpSelectionMerge %314 None
OpBranchConditional %311 %313 %314
%313 = OpLabel
%315 = OpLoad %mat3v2float %_3_m32
%316 = OpCompositeExtract %v2float %315 0
%317 = OpFOrdEqual %v2bool %316 %74
%318 = OpAll %bool %317
%319 = OpCompositeExtract %v2float %315 1
%320 = OpFOrdEqual %v2bool %319 %75
%321 = OpAll %bool %320
%322 = OpLogicalAnd %bool %318 %321
%323 = OpCompositeExtract %v2float %315 2
%324 = OpFOrdEqual %v2bool %323 %20
%325 = OpAll %bool %324
%326 = OpLogicalAnd %bool %322 %325
OpBranch %314
%314 = OpLabel
%327 = OpPhi %bool %false %302 %326 %313
OpStore %_0_ok %327
OpStore %_4_m34 %100
OpSelectionMerge %330 None
OpBranchConditional %327 %329 %330
%329 = OpLabel
%331 = OpLoad %mat3v4float %_4_m34
%332 = OpCompositeExtract %v4float %331 0
%333 = OpFOrdEqual %v4bool %332 %97
%334 = OpAll %bool %333
%335 = OpCompositeExtract %v4float %331 1
%336 = OpFOrdEqual %v4bool %335 %98
%337 = OpAll %bool %336
%338 = OpLogicalAnd %bool %334 %337
%339 = OpCompositeExtract %v4float %331 2
%340 = OpFOrdEqual %v4bool %339 %99
%341 = OpAll %bool %340
%342 = OpLogicalAnd %bool %338 %341
OpBranch %330
%330 = OpLabel
%343 = OpPhi %bool %false %314 %342 %329
OpStore %_0_ok %343
OpStore %_5_m42 %122
OpSelectionMerge %346 None
OpBranchConditional %343 %345 %346
%345 = OpLabel
%347 = OpLoad %mat4v2float %_5_m42
%348 = OpCompositeExtract %v2float %347 0
%349 = OpFOrdEqual %v2bool %348 %120
%350 = OpAll %bool %349
%351 = OpCompositeExtract %v2float %347 1
%352 = OpFOrdEqual %v2bool %351 %121
%353 = OpAll %bool %352
%354 = OpLogicalAnd %bool %350 %353
%355 = OpCompositeExtract %v2float %347 2
%356 = OpFOrdEqual %v2bool %355 %20
%357 = OpAll %bool %356
%358 = OpLogicalAnd %bool %354 %357
%359 = OpCompositeExtract %v2float %347 3
%360 = OpFOrdEqual %v2bool %359 %20
%361 = OpAll %bool %360
%362 = OpLogicalAnd %bool %358 %361
OpBranch %346
%346 = OpLabel
%363 = OpPhi %bool %false %330 %362 %345
OpStore %_0_ok %363
OpStore %_6_m43 %150
OpSelectionMerge %366 None
OpBranchConditional %363 %365 %366
%365 = OpLabel
%367 = OpLoad %mat4v3float %_6_m43
%368 = OpCompositeExtract %v3float %367 0
%369 = OpFOrdEqual %v3bool %368 %146
%370 = OpAll %bool %369
%371 = OpCompositeExtract %v3float %367 1
%372 = OpFOrdEqual %v3bool %371 %147
%373 = OpAll %bool %372
%374 = OpLogicalAnd %bool %370 %373
%375 = OpCompositeExtract %v3float %367 2
%376 = OpFOrdEqual %v3bool %375 %148
%377 = OpAll %bool %376
%378 = OpLogicalAnd %bool %374 %377
%379 = OpCompositeExtract %v3float %367 3
%380 = OpFOrdEqual %v3bool %379 %149
%381 = OpAll %bool %380
%382 = OpLogicalAnd %bool %378 %381
OpBranch %366
%366 = OpLabel
%383 = OpPhi %bool %false %346 %382 %365
OpStore %_0_ok %383
%385 = OpLoad %mat3v2float %_3_m32
%386 = OpLoad %mat2v3float %_1_m23
%387 = OpMatrixTimesMatrix %mat2v2float %385 %386
OpStore %_7_m22 %387
OpSelectionMerge %389 None
OpBranchConditional %383 %388 %389
%388 = OpLabel
%390 = OpLoad %mat2v2float %_7_m22
%391 = OpCompositeExtract %v2float %390 0
%392 = OpFOrdEqual %v2bool %391 %180
%393 = OpAll %bool %392
%394 = OpCompositeExtract %v2float %390 1
%395 = OpFOrdEqual %v2bool %394 %181
%396 = OpAll %bool %395
%397 = OpLogicalAnd %bool %393 %396
OpBranch %389
%389 = OpLabel
%398 = OpPhi %bool %false %366 %397 %388
OpStore %_0_ok %398
%400 = OpLoad %mat4v3float %_6_m43
%401 = OpLoad %mat3v4float %_4_m34
%402 = OpMatrixTimesMatrix %mat3v3float %400 %401
OpStore %_8_m33 %402
OpSelectionMerge %404 None
OpBranchConditional %398 %403 %404
%403 = OpLabel
%405 = OpLoad %mat3v3float %_8_m33
%406 = OpCompositeExtract %v3float %405 0
%407 = OpFOrdEqual %v3bool %406 %201
%408 = OpAll %bool %407
%409 = OpCompositeExtract %v3float %405 1
%410 = OpFOrdEqual %v3bool %409 %202
%411 = OpAll %bool %410
%412 = OpLogicalAnd %bool %408 %411
%413 = OpCompositeExtract %v3float %405 2
%414 = OpFOrdEqual %v3bool %413 %203
%415 = OpAll %bool %414
%416 = OpLogicalAnd %bool %412 %415
OpBranch %404
%404 = OpLabel
%417 = OpPhi %bool %false %389 %416 %403
OpStore %_0_ok %417
%418 = OpLoad %mat2v3float %_1_m23
%419 = OpCompositeExtract %v3float %418 0
%420 = OpFAdd %v3float %419 %219
%421 = OpCompositeExtract %v3float %418 1
%422 = OpFAdd %v3float %421 %219
%423 = OpCompositeConstruct %mat2v3float %420 %422
OpStore %_1_m23 %423
OpSelectionMerge %425 None
OpBranchConditional %417 %424 %425
%424 = OpLabel
%426 = OpFOrdEqual %v3bool %420 %228
%427 = OpAll %bool %426
%428 = OpFOrdEqual %v3bool %422 %229
%429 = OpAll %bool %428
%430 = OpLogicalAnd %bool %427 %429
OpBranch %425
%425 = OpLabel
%431 = OpPhi %bool %false %404 %430 %424
OpStore %_0_ok %431
%432 = OpLoad %mat3v2float %_3_m32
%433 = OpCompositeExtract %v2float %432 0
%434 = OpFSub %v2float %433 %238
%435 = OpCompositeExtract %v2float %432 1
%436 = OpFSub %v2float %435 %238
%437 = OpCompositeExtract %v2float %432 2
%438 = OpFSub %v2float %437 %238
%439 = OpCompositeConstruct %mat3v2float %434 %436 %438
OpStore %_3_m32 %439
OpSelectionMerge %441 None
OpBranchConditional %431 %440 %441
%440 = OpLabel
%442 = OpFOrdEqual %v2bool %434 %250
%443 = OpAll %bool %442
%444 = OpFOrdEqual %v2bool %436 %251
%445 = OpAll %bool %444
%446 = OpLogicalAnd %bool %443 %445
%447 = OpFOrdEqual %v2bool %438 %252
%448 = OpAll %bool %447
%449 = OpLogicalAnd %bool %446 %448
OpBranch %441
%441 = OpLabel
%450 = OpPhi %bool %false %425 %449 %440
OpStore %_0_ok %450
%451 = OpLoad %mat2v4float %_2_m24
%452 = OpCompositeExtract %v4float %451 0
%453 = OpFDiv %v4float %452 %264
%454 = OpCompositeExtract %v4float %451 1
%455 = OpFDiv %v4float %454 %264
%456 = OpCompositeConstruct %mat2v4float %453 %455
OpStore %_2_m24 %456
OpSelectionMerge %458 None
OpBranchConditional %450 %457 %458
%457 = OpLabel
%459 = OpFOrdEqual %v4bool %453 %274
%460 = OpAll %bool %459
%461 = OpFOrdEqual %v4bool %455 %275
%462 = OpAll %bool %461
%463 = OpLogicalAnd %bool %460 %462
OpBranch %458
%458 = OpLabel
%464 = OpPhi %bool %false %441 %463 %457
OpStore %_0_ok %464
OpSelectionMerge %466 None
OpBranchConditional %464 %465 %466
%465 = OpLabel
%467 = OpFunctionCall %bool %test_half_b
OpBranch %466
%466 = OpLabel
%468 = OpPhi %bool %false %458 %467 %465
OpSelectionMerge %473 None
OpBranchConditional %468 %471 %472
%471 = OpLabel
%474 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%478 = OpLoad %v4float %474
OpStore %469 %478
OpBranch %473
%472 = OpLabel
%479 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%481 = OpLoad %v4float %479
OpStore %469 %481
OpBranch %473
%473 = OpLabel
%482 = OpLoad %v4float %469
OpReturnValue %482
OpFunctionEnd
