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
OpDecorate %m24 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
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
%51 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%52 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%53 = OpConstantComposite %mat2v4float %51 %52
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%67 = OpConstantComposite %v2float %float_4 %float_0
%68 = OpConstantComposite %v2float %float_0 %float_4
%69 = OpConstantComposite %mat3v2float %67 %68 %20
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%86 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%87 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%88 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%89 = OpConstantComposite %mat3v4float %86 %87 %88
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%105 = OpConstantComposite %v2float %float_6 %float_0
%106 = OpConstantComposite %v2float %float_0 %float_6
%107 = OpConstantComposite %mat4v2float %105 %106 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%124 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%125 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%126 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%127 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%128 = OpConstantComposite %mat4v3float %124 %125 %126 %127
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%150 = OpConstantComposite %v2float %float_8 %float_0
%151 = OpConstantComposite %v2float %float_0 %float_8
%152 = OpConstantComposite %mat2v2float %150 %151
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%168 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%169 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%170 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%171 = OpConstantComposite %mat3v3float %168 %169 %170
%float_1 = OpConstant %float 1
%185 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%186 = OpConstantComposite %mat2v3float %185 %185
%192 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%193 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%194 = OpConstantComposite %mat2v3float %192 %193
%201 = OpConstantComposite %v2float %float_2 %float_2
%202 = OpConstantComposite %mat3v2float %201 %201 %201
%float_n2 = OpConstant %float -2
%210 = OpConstantComposite %v2float %float_2 %float_n2
%211 = OpConstantComposite %v2float %float_n2 %float_2
%212 = OpConstantComposite %v2float %float_n2 %float_n2
%213 = OpConstantComposite %mat3v2float %210 %211 %212
%223 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%224 = OpConstantComposite %mat2v4float %223 %223
%float_0_75 = OpConstant %float 0.75
%231 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%232 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%233 = OpConstantComposite %mat2v4float %231 %232
%240 = OpTypeFunction %v4float %_ptr_Function_v2float
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
OpSelectionMerge %39 None
OpBranchConditional %true %38 %39
%38 = OpLabel
%41 = OpFOrdEqual %v3bool %34 %34
%42 = OpAll %bool %41
%43 = OpFOrdEqual %v3bool %35 %35
%44 = OpAll %bool %43
%45 = OpLogicalAnd %bool %42 %44
OpBranch %39
%39 = OpLabel
%46 = OpPhi %bool %false %25 %45 %38
OpStore %ok %46
OpStore %m24 %53
OpSelectionMerge %55 None
OpBranchConditional %46 %54 %55
%54 = OpLabel
%57 = OpFOrdEqual %v4bool %51 %51
%58 = OpAll %bool %57
%59 = OpFOrdEqual %v4bool %52 %52
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %58 %60
OpBranch %55
%55 = OpLabel
%62 = OpPhi %bool %false %39 %61 %54
OpStore %ok %62
OpStore %m32 %69
OpSelectionMerge %71 None
OpBranchConditional %62 %70 %71
%70 = OpLabel
%73 = OpFOrdEqual %v2bool %67 %67
%74 = OpAll %bool %73
%75 = OpFOrdEqual %v2bool %68 %68
%76 = OpAll %bool %75
%77 = OpLogicalAnd %bool %74 %76
%78 = OpFOrdEqual %v2bool %20 %20
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %77 %79
OpBranch %71
%71 = OpLabel
%81 = OpPhi %bool %false %55 %80 %70
OpStore %ok %81
OpStore %m34 %89
OpSelectionMerge %91 None
OpBranchConditional %81 %90 %91
%90 = OpLabel
%92 = OpFOrdEqual %v4bool %86 %86
%93 = OpAll %bool %92
%94 = OpFOrdEqual %v4bool %87 %87
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %93 %95
%97 = OpFOrdEqual %v4bool %88 %88
%98 = OpAll %bool %97
%99 = OpLogicalAnd %bool %96 %98
OpBranch %91
%91 = OpLabel
%100 = OpPhi %bool %false %71 %99 %90
OpStore %ok %100
OpStore %m42 %107
OpSelectionMerge %109 None
OpBranchConditional %100 %108 %109
%108 = OpLabel
%110 = OpFOrdEqual %v2bool %105 %105
%111 = OpAll %bool %110
%112 = OpFOrdEqual %v2bool %106 %106
%113 = OpAll %bool %112
%114 = OpLogicalAnd %bool %111 %113
%115 = OpFOrdEqual %v2bool %20 %20
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %114 %116
%118 = OpLogicalAnd %bool %117 %116
OpBranch %109
%109 = OpLabel
%119 = OpPhi %bool %false %91 %118 %108
OpStore %ok %119
OpStore %m43 %128
OpSelectionMerge %130 None
OpBranchConditional %119 %129 %130
%129 = OpLabel
%131 = OpFOrdEqual %v3bool %124 %124
%132 = OpAll %bool %131
%133 = OpFOrdEqual %v3bool %125 %125
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %132 %134
%136 = OpFOrdEqual %v3bool %126 %126
%137 = OpAll %bool %136
%138 = OpLogicalAnd %bool %135 %137
%139 = OpFOrdEqual %v3bool %127 %127
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %138 %140
OpBranch %130
%130 = OpLabel
%142 = OpPhi %bool %false %109 %141 %129
OpStore %ok %142
%146 = OpMatrixTimesMatrix %mat2v2float %69 %36
OpStore %m22 %146
OpSelectionMerge %148 None
OpBranchConditional %142 %147 %148
%147 = OpLabel
%153 = OpCompositeExtract %v2float %146 0
%154 = OpFOrdEqual %v2bool %153 %150
%155 = OpAll %bool %154
%156 = OpCompositeExtract %v2float %146 1
%157 = OpFOrdEqual %v2bool %156 %151
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %155 %158
OpBranch %148
%148 = OpLabel
%160 = OpPhi %bool %false %130 %159 %147
OpStore %ok %160
%164 = OpMatrixTimesMatrix %mat3v3float %128 %89
OpStore %m33 %164
OpSelectionMerge %166 None
OpBranchConditional %160 %165 %166
%165 = OpLabel
%172 = OpCompositeExtract %v3float %164 0
%173 = OpFOrdEqual %v3bool %172 %168
%174 = OpAll %bool %173
%175 = OpCompositeExtract %v3float %164 1
%176 = OpFOrdEqual %v3bool %175 %169
%177 = OpAll %bool %176
%178 = OpLogicalAnd %bool %174 %177
%179 = OpCompositeExtract %v3float %164 2
%180 = OpFOrdEqual %v3bool %179 %170
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %178 %181
OpBranch %166
%166 = OpLabel
%183 = OpPhi %bool %false %148 %182 %165
OpStore %ok %183
%187 = OpFAdd %v3float %34 %185
%188 = OpFAdd %v3float %35 %185
%189 = OpCompositeConstruct %mat2v3float %187 %188
OpStore %m23 %189
OpSelectionMerge %191 None
OpBranchConditional %183 %190 %191
%190 = OpLabel
%195 = OpFOrdEqual %v3bool %187 %192
%196 = OpAll %bool %195
%197 = OpFOrdEqual %v3bool %188 %193
%198 = OpAll %bool %197
%199 = OpLogicalAnd %bool %196 %198
OpBranch %191
%191 = OpLabel
%200 = OpPhi %bool %false %166 %199 %190
OpStore %ok %200
%203 = OpFSub %v2float %67 %201
%204 = OpFSub %v2float %68 %201
%205 = OpFSub %v2float %20 %201
%206 = OpCompositeConstruct %mat3v2float %203 %204 %205
OpStore %m32 %206
OpSelectionMerge %208 None
OpBranchConditional %200 %207 %208
%207 = OpLabel
%214 = OpFOrdEqual %v2bool %203 %210
%215 = OpAll %bool %214
%216 = OpFOrdEqual %v2bool %204 %211
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %215 %217
%219 = OpFOrdEqual %v2bool %205 %212
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %218 %220
OpBranch %208
%208 = OpLabel
%222 = OpPhi %bool %false %191 %221 %207
OpStore %ok %222
%225 = OpFDiv %v4float %51 %223
%226 = OpFDiv %v4float %52 %223
%227 = OpCompositeConstruct %mat2v4float %225 %226
OpStore %m24 %227
OpSelectionMerge %229 None
OpBranchConditional %222 %228 %229
%228 = OpLabel
%234 = OpFOrdEqual %v4bool %225 %231
%235 = OpAll %bool %234
%236 = OpFOrdEqual %v4bool %226 %232
%237 = OpAll %bool %236
%238 = OpLogicalAnd %bool %235 %237
OpBranch %229
%229 = OpLabel
%239 = OpPhi %bool %false %208 %238 %228
OpStore %ok %239
OpReturnValue %239
OpFunctionEnd
%main = OpFunction %v4float None %240
%241 = OpFunctionParameter %_ptr_Function_v2float
%242 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%383 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m23 %36
OpSelectionMerge %246 None
OpBranchConditional %true %245 %246
%245 = OpLabel
%247 = OpFOrdEqual %v3bool %34 %34
%248 = OpAll %bool %247
%249 = OpFOrdEqual %v3bool %35 %35
%250 = OpAll %bool %249
%251 = OpLogicalAnd %bool %248 %250
OpBranch %246
%246 = OpLabel
%252 = OpPhi %bool %false %242 %251 %245
OpStore %_0_ok %252
OpStore %_2_m24 %53
OpSelectionMerge %255 None
OpBranchConditional %252 %254 %255
%254 = OpLabel
%256 = OpFOrdEqual %v4bool %51 %51
%257 = OpAll %bool %256
%258 = OpFOrdEqual %v4bool %52 %52
%259 = OpAll %bool %258
%260 = OpLogicalAnd %bool %257 %259
OpBranch %255
%255 = OpLabel
%261 = OpPhi %bool %false %246 %260 %254
OpStore %_0_ok %261
OpStore %_3_m32 %69
OpSelectionMerge %264 None
OpBranchConditional %261 %263 %264
%263 = OpLabel
%265 = OpFOrdEqual %v2bool %67 %67
%266 = OpAll %bool %265
%267 = OpFOrdEqual %v2bool %68 %68
%268 = OpAll %bool %267
%269 = OpLogicalAnd %bool %266 %268
%270 = OpFOrdEqual %v2bool %20 %20
%271 = OpAll %bool %270
%272 = OpLogicalAnd %bool %269 %271
OpBranch %264
%264 = OpLabel
%273 = OpPhi %bool %false %255 %272 %263
OpStore %_0_ok %273
OpStore %_4_m34 %89
OpSelectionMerge %276 None
OpBranchConditional %273 %275 %276
%275 = OpLabel
%277 = OpFOrdEqual %v4bool %86 %86
%278 = OpAll %bool %277
%279 = OpFOrdEqual %v4bool %87 %87
%280 = OpAll %bool %279
%281 = OpLogicalAnd %bool %278 %280
%282 = OpFOrdEqual %v4bool %88 %88
%283 = OpAll %bool %282
%284 = OpLogicalAnd %bool %281 %283
OpBranch %276
%276 = OpLabel
%285 = OpPhi %bool %false %264 %284 %275
OpStore %_0_ok %285
OpStore %_5_m42 %107
OpSelectionMerge %288 None
OpBranchConditional %285 %287 %288
%287 = OpLabel
%289 = OpFOrdEqual %v2bool %105 %105
%290 = OpAll %bool %289
%291 = OpFOrdEqual %v2bool %106 %106
%292 = OpAll %bool %291
%293 = OpLogicalAnd %bool %290 %292
%294 = OpFOrdEqual %v2bool %20 %20
%295 = OpAll %bool %294
%296 = OpLogicalAnd %bool %293 %295
%297 = OpLogicalAnd %bool %296 %295
OpBranch %288
%288 = OpLabel
%298 = OpPhi %bool %false %276 %297 %287
OpStore %_0_ok %298
OpStore %_6_m43 %128
OpSelectionMerge %301 None
OpBranchConditional %298 %300 %301
%300 = OpLabel
%302 = OpFOrdEqual %v3bool %124 %124
%303 = OpAll %bool %302
%304 = OpFOrdEqual %v3bool %125 %125
%305 = OpAll %bool %304
%306 = OpLogicalAnd %bool %303 %305
%307 = OpFOrdEqual %v3bool %126 %126
%308 = OpAll %bool %307
%309 = OpLogicalAnd %bool %306 %308
%310 = OpFOrdEqual %v3bool %127 %127
%311 = OpAll %bool %310
%312 = OpLogicalAnd %bool %309 %311
OpBranch %301
%301 = OpLabel
%313 = OpPhi %bool %false %288 %312 %300
OpStore %_0_ok %313
%315 = OpMatrixTimesMatrix %mat2v2float %69 %36
OpStore %_7_m22 %315
OpSelectionMerge %317 None
OpBranchConditional %313 %316 %317
%316 = OpLabel
%318 = OpCompositeExtract %v2float %315 0
%319 = OpFOrdEqual %v2bool %318 %150
%320 = OpAll %bool %319
%321 = OpCompositeExtract %v2float %315 1
%322 = OpFOrdEqual %v2bool %321 %151
%323 = OpAll %bool %322
%324 = OpLogicalAnd %bool %320 %323
OpBranch %317
%317 = OpLabel
%325 = OpPhi %bool %false %301 %324 %316
OpStore %_0_ok %325
%327 = OpMatrixTimesMatrix %mat3v3float %128 %89
OpStore %_8_m33 %327
OpSelectionMerge %329 None
OpBranchConditional %325 %328 %329
%328 = OpLabel
%330 = OpCompositeExtract %v3float %327 0
%331 = OpFOrdEqual %v3bool %330 %168
%332 = OpAll %bool %331
%333 = OpCompositeExtract %v3float %327 1
%334 = OpFOrdEqual %v3bool %333 %169
%335 = OpAll %bool %334
%336 = OpLogicalAnd %bool %332 %335
%337 = OpCompositeExtract %v3float %327 2
%338 = OpFOrdEqual %v3bool %337 %170
%339 = OpAll %bool %338
%340 = OpLogicalAnd %bool %336 %339
OpBranch %329
%329 = OpLabel
%341 = OpPhi %bool %false %317 %340 %328
OpStore %_0_ok %341
%342 = OpFAdd %v3float %34 %185
%343 = OpFAdd %v3float %35 %185
%344 = OpCompositeConstruct %mat2v3float %342 %343
OpStore %_1_m23 %344
OpSelectionMerge %346 None
OpBranchConditional %341 %345 %346
%345 = OpLabel
%347 = OpFOrdEqual %v3bool %342 %192
%348 = OpAll %bool %347
%349 = OpFOrdEqual %v3bool %343 %193
%350 = OpAll %bool %349
%351 = OpLogicalAnd %bool %348 %350
OpBranch %346
%346 = OpLabel
%352 = OpPhi %bool %false %329 %351 %345
OpStore %_0_ok %352
%353 = OpFSub %v2float %67 %201
%354 = OpFSub %v2float %68 %201
%355 = OpFSub %v2float %20 %201
%356 = OpCompositeConstruct %mat3v2float %353 %354 %355
OpStore %_3_m32 %356
OpSelectionMerge %358 None
OpBranchConditional %352 %357 %358
%357 = OpLabel
%359 = OpFOrdEqual %v2bool %353 %210
%360 = OpAll %bool %359
%361 = OpFOrdEqual %v2bool %354 %211
%362 = OpAll %bool %361
%363 = OpLogicalAnd %bool %360 %362
%364 = OpFOrdEqual %v2bool %355 %212
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %363 %365
OpBranch %358
%358 = OpLabel
%367 = OpPhi %bool %false %346 %366 %357
OpStore %_0_ok %367
%368 = OpFDiv %v4float %51 %223
%369 = OpFDiv %v4float %52 %223
%370 = OpCompositeConstruct %mat2v4float %368 %369
OpStore %_2_m24 %370
OpSelectionMerge %372 None
OpBranchConditional %367 %371 %372
%371 = OpLabel
%373 = OpFOrdEqual %v4bool %368 %231
%374 = OpAll %bool %373
%375 = OpFOrdEqual %v4bool %369 %232
%376 = OpAll %bool %375
%377 = OpLogicalAnd %bool %374 %376
OpBranch %372
%372 = OpLabel
%378 = OpPhi %bool %false %358 %377 %371
OpStore %_0_ok %378
OpSelectionMerge %380 None
OpBranchConditional %378 %379 %380
%379 = OpLabel
%381 = OpFunctionCall %bool %test_half_b
OpBranch %380
%380 = OpLabel
%382 = OpPhi %bool %false %372 %381 %379
OpSelectionMerge %387 None
OpBranchConditional %382 %385 %386
%385 = OpLabel
%388 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%392 = OpLoad %v4float %388
OpStore %383 %392
OpBranch %387
%386 = OpLabel
%393 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%395 = OpLoad %v4float %393
OpStore %383 %395
OpBranch %387
%387 = OpLabel
%396 = OpLoad %v4float %383
OpReturnValue %396
OpFunctionEnd
