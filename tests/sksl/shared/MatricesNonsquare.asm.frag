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
OpDecorate %40 RelaxedPrecision
OpDecorate %m24 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %m32 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %m34 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %m42 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %m43 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %m22 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %m33 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %496 RelaxedPrecision
OpDecorate %499 RelaxedPrecision
OpDecorate %500 RelaxedPrecision
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
%54 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
%55 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
%56 = OpConstantComposite %mat2v4float %54 %55
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%73 = OpConstantComposite %v2float %float_4 %float_0
%74 = OpConstantComposite %v2float %float_0 %float_4
%75 = OpConstantComposite %mat3v2float %73 %74 %20
%v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%96 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
%97 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
%98 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
%99 = OpConstantComposite %mat3v4float %96 %97 %98
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%119 = OpConstantComposite %v2float %float_6 %float_0
%120 = OpConstantComposite %v2float %float_0 %float_6
%121 = OpConstantComposite %mat4v2float %119 %120 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%145 = OpConstantComposite %v3float %float_7 %float_0 %float_0
%146 = OpConstantComposite %v3float %float_0 %float_7 %float_0
%147 = OpConstantComposite %v3float %float_0 %float_0 %float_7
%148 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%149 = OpConstantComposite %mat4v3float %145 %146 %147 %148
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_8 = OpConstant %float 8
%179 = OpConstantComposite %v2float %float_8 %float_0
%180 = OpConstantComposite %v2float %float_0 %float_8
%181 = OpConstantComposite %mat2v2float %179 %180
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%200 = OpConstantComposite %v3float %float_35 %float_0 %float_0
%201 = OpConstantComposite %v3float %float_0 %float_35 %float_0
%202 = OpConstantComposite %v3float %float_0 %float_0 %float_35
%203 = OpConstantComposite %mat3v3float %200 %201 %202
%float_1 = OpConstant %float 1
%218 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%219 = OpConstantComposite %mat2v3float %218 %218
%228 = OpConstantComposite %v3float %float_3 %float_1 %float_1
%229 = OpConstantComposite %v3float %float_1 %float_3 %float_1
%230 = OpConstantComposite %mat2v3float %228 %229
%240 = OpConstantComposite %v2float %float_2 %float_2
%241 = OpConstantComposite %mat3v2float %240 %240 %240
%float_n2 = OpConstant %float -2
%253 = OpConstantComposite %v2float %float_2 %float_n2
%254 = OpConstantComposite %v2float %float_n2 %float_2
%255 = OpConstantComposite %v2float %float_n2 %float_n2
%256 = OpConstantComposite %mat3v2float %253 %254 %255
%270 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%271 = OpConstantComposite %mat2v4float %270 %270
%float_0_75 = OpConstant %float 0.75
%281 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
%282 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
%283 = OpConstantComposite %mat2v4float %281 %282
%292 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%40 = OpLoad %mat2v3float %m23
%42 = OpCompositeExtract %v3float %40 0
%43 = OpFOrdEqual %v3bool %42 %34
%44 = OpAll %bool %43
%45 = OpCompositeExtract %v3float %40 1
%46 = OpFOrdEqual %v3bool %45 %35
%47 = OpAll %bool %46
%48 = OpLogicalAnd %bool %44 %47
OpBranch %39
%39 = OpLabel
%49 = OpPhi %bool %false %25 %48 %38
OpStore %ok %49
OpStore %m24 %56
OpSelectionMerge %58 None
OpBranchConditional %49 %57 %58
%57 = OpLabel
%59 = OpLoad %mat2v4float %m24
%61 = OpCompositeExtract %v4float %59 0
%62 = OpFOrdEqual %v4bool %61 %54
%63 = OpAll %bool %62
%64 = OpCompositeExtract %v4float %59 1
%65 = OpFOrdEqual %v4bool %64 %55
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %63 %66
OpBranch %58
%58 = OpLabel
%68 = OpPhi %bool %false %39 %67 %57
OpStore %ok %68
OpStore %m32 %75
OpSelectionMerge %77 None
OpBranchConditional %68 %76 %77
%76 = OpLabel
%78 = OpLoad %mat3v2float %m32
%80 = OpCompositeExtract %v2float %78 0
%81 = OpFOrdEqual %v2bool %80 %73
%82 = OpAll %bool %81
%83 = OpCompositeExtract %v2float %78 1
%84 = OpFOrdEqual %v2bool %83 %74
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %82 %85
%87 = OpCompositeExtract %v2float %78 2
%88 = OpFOrdEqual %v2bool %87 %20
%89 = OpAll %bool %88
%90 = OpLogicalAnd %bool %86 %89
OpBranch %77
%77 = OpLabel
%91 = OpPhi %bool %false %58 %90 %76
OpStore %ok %91
OpStore %m34 %99
OpSelectionMerge %101 None
OpBranchConditional %91 %100 %101
%100 = OpLabel
%102 = OpLoad %mat3v4float %m34
%103 = OpCompositeExtract %v4float %102 0
%104 = OpFOrdEqual %v4bool %103 %96
%105 = OpAll %bool %104
%106 = OpCompositeExtract %v4float %102 1
%107 = OpFOrdEqual %v4bool %106 %97
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %105 %108
%110 = OpCompositeExtract %v4float %102 2
%111 = OpFOrdEqual %v4bool %110 %98
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %109 %112
OpBranch %101
%101 = OpLabel
%114 = OpPhi %bool %false %77 %113 %100
OpStore %ok %114
OpStore %m42 %121
OpSelectionMerge %123 None
OpBranchConditional %114 %122 %123
%122 = OpLabel
%124 = OpLoad %mat4v2float %m42
%125 = OpCompositeExtract %v2float %124 0
%126 = OpFOrdEqual %v2bool %125 %119
%127 = OpAll %bool %126
%128 = OpCompositeExtract %v2float %124 1
%129 = OpFOrdEqual %v2bool %128 %120
%130 = OpAll %bool %129
%131 = OpLogicalAnd %bool %127 %130
%132 = OpCompositeExtract %v2float %124 2
%133 = OpFOrdEqual %v2bool %132 %20
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %131 %134
%136 = OpCompositeExtract %v2float %124 3
%137 = OpFOrdEqual %v2bool %136 %20
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %135 %138
OpBranch %123
%123 = OpLabel
%140 = OpPhi %bool %false %101 %139 %122
OpStore %ok %140
OpStore %m43 %149
OpSelectionMerge %151 None
OpBranchConditional %140 %150 %151
%150 = OpLabel
%152 = OpLoad %mat4v3float %m43
%153 = OpCompositeExtract %v3float %152 0
%154 = OpFOrdEqual %v3bool %153 %145
%155 = OpAll %bool %154
%156 = OpCompositeExtract %v3float %152 1
%157 = OpFOrdEqual %v3bool %156 %146
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %155 %158
%160 = OpCompositeExtract %v3float %152 2
%161 = OpFOrdEqual %v3bool %160 %147
%162 = OpAll %bool %161
%163 = OpLogicalAnd %bool %159 %162
%164 = OpCompositeExtract %v3float %152 3
%165 = OpFOrdEqual %v3bool %164 %148
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %163 %166
OpBranch %151
%151 = OpLabel
%168 = OpPhi %bool %false %123 %167 %150
OpStore %ok %168
%172 = OpLoad %mat3v2float %m32
%173 = OpLoad %mat2v3float %m23
%174 = OpMatrixTimesMatrix %mat2v2float %172 %173
OpStore %m22 %174
OpSelectionMerge %176 None
OpBranchConditional %168 %175 %176
%175 = OpLabel
%177 = OpLoad %mat2v2float %m22
%182 = OpCompositeExtract %v2float %177 0
%183 = OpFOrdEqual %v2bool %182 %179
%184 = OpAll %bool %183
%185 = OpCompositeExtract %v2float %177 1
%186 = OpFOrdEqual %v2bool %185 %180
%187 = OpAll %bool %186
%188 = OpLogicalAnd %bool %184 %187
OpBranch %176
%176 = OpLabel
%189 = OpPhi %bool %false %151 %188 %175
OpStore %ok %189
%193 = OpLoad %mat4v3float %m43
%194 = OpLoad %mat3v4float %m34
%195 = OpMatrixTimesMatrix %mat3v3float %193 %194
OpStore %m33 %195
OpSelectionMerge %197 None
OpBranchConditional %189 %196 %197
%196 = OpLabel
%198 = OpLoad %mat3v3float %m33
%204 = OpCompositeExtract %v3float %198 0
%205 = OpFOrdEqual %v3bool %204 %200
%206 = OpAll %bool %205
%207 = OpCompositeExtract %v3float %198 1
%208 = OpFOrdEqual %v3bool %207 %201
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %206 %209
%211 = OpCompositeExtract %v3float %198 2
%212 = OpFOrdEqual %v3bool %211 %202
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %210 %213
OpBranch %197
%197 = OpLabel
%215 = OpPhi %bool %false %176 %214 %196
OpStore %ok %215
%216 = OpLoad %mat2v3float %m23
%220 = OpCompositeExtract %v3float %216 0
%221 = OpFAdd %v3float %220 %218
%222 = OpCompositeExtract %v3float %216 1
%223 = OpFAdd %v3float %222 %218
%224 = OpCompositeConstruct %mat2v3float %221 %223
OpStore %m23 %224
OpSelectionMerge %226 None
OpBranchConditional %215 %225 %226
%225 = OpLabel
%227 = OpLoad %mat2v3float %m23
%231 = OpCompositeExtract %v3float %227 0
%232 = OpFOrdEqual %v3bool %231 %228
%233 = OpAll %bool %232
%234 = OpCompositeExtract %v3float %227 1
%235 = OpFOrdEqual %v3bool %234 %229
%236 = OpAll %bool %235
%237 = OpLogicalAnd %bool %233 %236
OpBranch %226
%226 = OpLabel
%238 = OpPhi %bool %false %197 %237 %225
OpStore %ok %238
%239 = OpLoad %mat3v2float %m32
%242 = OpCompositeExtract %v2float %239 0
%243 = OpFSub %v2float %242 %240
%244 = OpCompositeExtract %v2float %239 1
%245 = OpFSub %v2float %244 %240
%246 = OpCompositeExtract %v2float %239 2
%247 = OpFSub %v2float %246 %240
%248 = OpCompositeConstruct %mat3v2float %243 %245 %247
OpStore %m32 %248
OpSelectionMerge %250 None
OpBranchConditional %238 %249 %250
%249 = OpLabel
%251 = OpLoad %mat3v2float %m32
%257 = OpCompositeExtract %v2float %251 0
%258 = OpFOrdEqual %v2bool %257 %253
%259 = OpAll %bool %258
%260 = OpCompositeExtract %v2float %251 1
%261 = OpFOrdEqual %v2bool %260 %254
%262 = OpAll %bool %261
%263 = OpLogicalAnd %bool %259 %262
%264 = OpCompositeExtract %v2float %251 2
%265 = OpFOrdEqual %v2bool %264 %255
%266 = OpAll %bool %265
%267 = OpLogicalAnd %bool %263 %266
OpBranch %250
%250 = OpLabel
%268 = OpPhi %bool %false %226 %267 %249
OpStore %ok %268
%269 = OpLoad %mat2v4float %m24
%272 = OpCompositeExtract %v4float %269 0
%273 = OpFDiv %v4float %272 %270
%274 = OpCompositeExtract %v4float %269 1
%275 = OpFDiv %v4float %274 %270
%276 = OpCompositeConstruct %mat2v4float %273 %275
OpStore %m24 %276
OpSelectionMerge %278 None
OpBranchConditional %268 %277 %278
%277 = OpLabel
%279 = OpLoad %mat2v4float %m24
%284 = OpCompositeExtract %v4float %279 0
%285 = OpFOrdEqual %v4bool %284 %281
%286 = OpAll %bool %285
%287 = OpCompositeExtract %v4float %279 1
%288 = OpFOrdEqual %v4bool %287 %282
%289 = OpAll %bool %288
%290 = OpLogicalAnd %bool %286 %289
OpBranch %278
%278 = OpLabel
%291 = OpPhi %bool %false %250 %290 %277
OpStore %ok %291
OpReturnValue %291
OpFunctionEnd
%main = OpFunction %v4float None %292
%293 = OpFunctionParameter %_ptr_Function_v2float
%294 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%487 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m23 %36
OpSelectionMerge %298 None
OpBranchConditional %true %297 %298
%297 = OpLabel
%299 = OpLoad %mat2v3float %_1_m23
%300 = OpCompositeExtract %v3float %299 0
%301 = OpFOrdEqual %v3bool %300 %34
%302 = OpAll %bool %301
%303 = OpCompositeExtract %v3float %299 1
%304 = OpFOrdEqual %v3bool %303 %35
%305 = OpAll %bool %304
%306 = OpLogicalAnd %bool %302 %305
OpBranch %298
%298 = OpLabel
%307 = OpPhi %bool %false %294 %306 %297
OpStore %_0_ok %307
OpStore %_2_m24 %56
OpSelectionMerge %310 None
OpBranchConditional %307 %309 %310
%309 = OpLabel
%311 = OpLoad %mat2v4float %_2_m24
%312 = OpCompositeExtract %v4float %311 0
%313 = OpFOrdEqual %v4bool %312 %54
%314 = OpAll %bool %313
%315 = OpCompositeExtract %v4float %311 1
%316 = OpFOrdEqual %v4bool %315 %55
%317 = OpAll %bool %316
%318 = OpLogicalAnd %bool %314 %317
OpBranch %310
%310 = OpLabel
%319 = OpPhi %bool %false %298 %318 %309
OpStore %_0_ok %319
OpStore %_3_m32 %75
OpSelectionMerge %322 None
OpBranchConditional %319 %321 %322
%321 = OpLabel
%323 = OpLoad %mat3v2float %_3_m32
%324 = OpCompositeExtract %v2float %323 0
%325 = OpFOrdEqual %v2bool %324 %73
%326 = OpAll %bool %325
%327 = OpCompositeExtract %v2float %323 1
%328 = OpFOrdEqual %v2bool %327 %74
%329 = OpAll %bool %328
%330 = OpLogicalAnd %bool %326 %329
%331 = OpCompositeExtract %v2float %323 2
%332 = OpFOrdEqual %v2bool %331 %20
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %330 %333
OpBranch %322
%322 = OpLabel
%335 = OpPhi %bool %false %310 %334 %321
OpStore %_0_ok %335
OpStore %_4_m34 %99
OpSelectionMerge %338 None
OpBranchConditional %335 %337 %338
%337 = OpLabel
%339 = OpLoad %mat3v4float %_4_m34
%340 = OpCompositeExtract %v4float %339 0
%341 = OpFOrdEqual %v4bool %340 %96
%342 = OpAll %bool %341
%343 = OpCompositeExtract %v4float %339 1
%344 = OpFOrdEqual %v4bool %343 %97
%345 = OpAll %bool %344
%346 = OpLogicalAnd %bool %342 %345
%347 = OpCompositeExtract %v4float %339 2
%348 = OpFOrdEqual %v4bool %347 %98
%349 = OpAll %bool %348
%350 = OpLogicalAnd %bool %346 %349
OpBranch %338
%338 = OpLabel
%351 = OpPhi %bool %false %322 %350 %337
OpStore %_0_ok %351
OpStore %_5_m42 %121
OpSelectionMerge %354 None
OpBranchConditional %351 %353 %354
%353 = OpLabel
%355 = OpLoad %mat4v2float %_5_m42
%356 = OpCompositeExtract %v2float %355 0
%357 = OpFOrdEqual %v2bool %356 %119
%358 = OpAll %bool %357
%359 = OpCompositeExtract %v2float %355 1
%360 = OpFOrdEqual %v2bool %359 %120
%361 = OpAll %bool %360
%362 = OpLogicalAnd %bool %358 %361
%363 = OpCompositeExtract %v2float %355 2
%364 = OpFOrdEqual %v2bool %363 %20
%365 = OpAll %bool %364
%366 = OpLogicalAnd %bool %362 %365
%367 = OpCompositeExtract %v2float %355 3
%368 = OpFOrdEqual %v2bool %367 %20
%369 = OpAll %bool %368
%370 = OpLogicalAnd %bool %366 %369
OpBranch %354
%354 = OpLabel
%371 = OpPhi %bool %false %338 %370 %353
OpStore %_0_ok %371
OpStore %_6_m43 %149
OpSelectionMerge %374 None
OpBranchConditional %371 %373 %374
%373 = OpLabel
%375 = OpLoad %mat4v3float %_6_m43
%376 = OpCompositeExtract %v3float %375 0
%377 = OpFOrdEqual %v3bool %376 %145
%378 = OpAll %bool %377
%379 = OpCompositeExtract %v3float %375 1
%380 = OpFOrdEqual %v3bool %379 %146
%381 = OpAll %bool %380
%382 = OpLogicalAnd %bool %378 %381
%383 = OpCompositeExtract %v3float %375 2
%384 = OpFOrdEqual %v3bool %383 %147
%385 = OpAll %bool %384
%386 = OpLogicalAnd %bool %382 %385
%387 = OpCompositeExtract %v3float %375 3
%388 = OpFOrdEqual %v3bool %387 %148
%389 = OpAll %bool %388
%390 = OpLogicalAnd %bool %386 %389
OpBranch %374
%374 = OpLabel
%391 = OpPhi %bool %false %354 %390 %373
OpStore %_0_ok %391
%393 = OpLoad %mat3v2float %_3_m32
%394 = OpLoad %mat2v3float %_1_m23
%395 = OpMatrixTimesMatrix %mat2v2float %393 %394
OpStore %_7_m22 %395
OpSelectionMerge %397 None
OpBranchConditional %391 %396 %397
%396 = OpLabel
%398 = OpLoad %mat2v2float %_7_m22
%399 = OpCompositeExtract %v2float %398 0
%400 = OpFOrdEqual %v2bool %399 %179
%401 = OpAll %bool %400
%402 = OpCompositeExtract %v2float %398 1
%403 = OpFOrdEqual %v2bool %402 %180
%404 = OpAll %bool %403
%405 = OpLogicalAnd %bool %401 %404
OpBranch %397
%397 = OpLabel
%406 = OpPhi %bool %false %374 %405 %396
OpStore %_0_ok %406
%408 = OpLoad %mat4v3float %_6_m43
%409 = OpLoad %mat3v4float %_4_m34
%410 = OpMatrixTimesMatrix %mat3v3float %408 %409
OpStore %_8_m33 %410
OpSelectionMerge %412 None
OpBranchConditional %406 %411 %412
%411 = OpLabel
%413 = OpLoad %mat3v3float %_8_m33
%414 = OpCompositeExtract %v3float %413 0
%415 = OpFOrdEqual %v3bool %414 %200
%416 = OpAll %bool %415
%417 = OpCompositeExtract %v3float %413 1
%418 = OpFOrdEqual %v3bool %417 %201
%419 = OpAll %bool %418
%420 = OpLogicalAnd %bool %416 %419
%421 = OpCompositeExtract %v3float %413 2
%422 = OpFOrdEqual %v3bool %421 %202
%423 = OpAll %bool %422
%424 = OpLogicalAnd %bool %420 %423
OpBranch %412
%412 = OpLabel
%425 = OpPhi %bool %false %397 %424 %411
OpStore %_0_ok %425
%426 = OpLoad %mat2v3float %_1_m23
%427 = OpCompositeExtract %v3float %426 0
%428 = OpFAdd %v3float %427 %218
%429 = OpCompositeExtract %v3float %426 1
%430 = OpFAdd %v3float %429 %218
%431 = OpCompositeConstruct %mat2v3float %428 %430
OpStore %_1_m23 %431
OpSelectionMerge %433 None
OpBranchConditional %425 %432 %433
%432 = OpLabel
%434 = OpLoad %mat2v3float %_1_m23
%435 = OpCompositeExtract %v3float %434 0
%436 = OpFOrdEqual %v3bool %435 %228
%437 = OpAll %bool %436
%438 = OpCompositeExtract %v3float %434 1
%439 = OpFOrdEqual %v3bool %438 %229
%440 = OpAll %bool %439
%441 = OpLogicalAnd %bool %437 %440
OpBranch %433
%433 = OpLabel
%442 = OpPhi %bool %false %412 %441 %432
OpStore %_0_ok %442
%443 = OpLoad %mat3v2float %_3_m32
%444 = OpCompositeExtract %v2float %443 0
%445 = OpFSub %v2float %444 %240
%446 = OpCompositeExtract %v2float %443 1
%447 = OpFSub %v2float %446 %240
%448 = OpCompositeExtract %v2float %443 2
%449 = OpFSub %v2float %448 %240
%450 = OpCompositeConstruct %mat3v2float %445 %447 %449
OpStore %_3_m32 %450
OpSelectionMerge %452 None
OpBranchConditional %442 %451 %452
%451 = OpLabel
%453 = OpLoad %mat3v2float %_3_m32
%454 = OpCompositeExtract %v2float %453 0
%455 = OpFOrdEqual %v2bool %454 %253
%456 = OpAll %bool %455
%457 = OpCompositeExtract %v2float %453 1
%458 = OpFOrdEqual %v2bool %457 %254
%459 = OpAll %bool %458
%460 = OpLogicalAnd %bool %456 %459
%461 = OpCompositeExtract %v2float %453 2
%462 = OpFOrdEqual %v2bool %461 %255
%463 = OpAll %bool %462
%464 = OpLogicalAnd %bool %460 %463
OpBranch %452
%452 = OpLabel
%465 = OpPhi %bool %false %433 %464 %451
OpStore %_0_ok %465
%466 = OpLoad %mat2v4float %_2_m24
%467 = OpCompositeExtract %v4float %466 0
%468 = OpFDiv %v4float %467 %270
%469 = OpCompositeExtract %v4float %466 1
%470 = OpFDiv %v4float %469 %270
%471 = OpCompositeConstruct %mat2v4float %468 %470
OpStore %_2_m24 %471
OpSelectionMerge %473 None
OpBranchConditional %465 %472 %473
%472 = OpLabel
%474 = OpLoad %mat2v4float %_2_m24
%475 = OpCompositeExtract %v4float %474 0
%476 = OpFOrdEqual %v4bool %475 %281
%477 = OpAll %bool %476
%478 = OpCompositeExtract %v4float %474 1
%479 = OpFOrdEqual %v4bool %478 %282
%480 = OpAll %bool %479
%481 = OpLogicalAnd %bool %477 %480
OpBranch %473
%473 = OpLabel
%482 = OpPhi %bool %false %452 %481 %472
OpStore %_0_ok %482
OpSelectionMerge %484 None
OpBranchConditional %482 %483 %484
%483 = OpLabel
%485 = OpFunctionCall %bool %test_half_b
OpBranch %484
%484 = OpLabel
%486 = OpPhi %bool %false %473 %485 %483
OpSelectionMerge %491 None
OpBranchConditional %486 %489 %490
%489 = OpLabel
%492 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%496 = OpLoad %v4float %492
OpStore %487 %496
OpBranch %491
%490 = OpLabel
%497 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%499 = OpLoad %v4float %497
OpStore %487 %499
OpBranch %491
%491 = OpLabel
%500 = OpLoad %v4float %487
OpReturnValue %500
OpFunctionEnd
