OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_int_b "test_int_b"
OpName %ok "ok"
OpName %inputRed "inputRed"
OpName %inputGreen "inputGreen"
OpName %x "x"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_inputRed "_1_inputRed"
OpName %_2_inputGreen "_2_inputGreen"
OpName %_3_x "_3_x"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%61 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%false = OpConstantFalse %bool
%int_3 = OpConstant %int 3
%68 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%81 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%93 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%101 = OpConstantComposite %v3int %int_9 %int_9 %int_9
%108 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%115 = OpConstantComposite %v2int %int_3 %int_3
%122 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%128 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
%134 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%149 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
%154 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%166 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%184 = OpConstantComposite %v2int %int_36 %int_36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%193 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%198 = OpConstantComposite %v4int %int_36 %int_36 %int_36 %int_36
%204 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%211 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%231 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%244 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_3 = OpConstant %float 3
%250 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%262 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%275 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%289 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%303 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%314 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%329 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%334 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%346 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%364 = OpConstantComposite %v2float %float_36 %float_36
%371 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%376 = OpConstantComposite %v4float %float_36 %float_36 %float_36 %float_36
%382 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%389 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%float_0_5 = OpConstant %float 0.5
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%inputRed = OpVariable %_ptr_Function_v4int Function
%inputGreen = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_v4int Function
OpStore %ok %true
%33 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %33
%37 = OpCompositeExtract %float %36 0
%38 = OpConvertFToS %int %37
%39 = OpCompositeExtract %float %36 1
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %36 2
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %36 3
%44 = OpConvertFToS %int %43
%45 = OpCompositeConstruct %v4int %38 %40 %42 %44
OpStore %inputRed %45
%47 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%49 = OpLoad %v4float %47
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %inputGreen %58
%62 = OpIAdd %v4int %45 %61
OpStore %x %62
OpSelectionMerge %65 None
OpBranchConditional %true %64 %65
%64 = OpLabel
%66 = OpLoad %v4int %x
%69 = OpIEqual %v4bool %66 %68
%71 = OpAll %bool %69
OpBranch %65
%65 = OpLabel
%72 = OpPhi %bool %false %25 %71 %64
OpStore %ok %72
%73 = OpLoad %v4int %inputGreen
%74 = OpVectorShuffle %v4int %73 %73 1 3 0 2
%75 = OpISub %v4int %74 %61
OpStore %x %75
OpSelectionMerge %77 None
OpBranchConditional %72 %76 %77
%76 = OpLabel
%78 = OpLoad %v4int %x
%82 = OpIEqual %v4bool %78 %81
%83 = OpAll %bool %82
OpBranch %77
%77 = OpLabel
%84 = OpPhi %bool %false %65 %83 %76
OpStore %ok %84
%85 = OpLoad %v4int %inputRed
%86 = OpLoad %v4int %inputGreen
%87 = OpCompositeExtract %int %86 1
%88 = OpCompositeConstruct %v4int %87 %87 %87 %87
%89 = OpIAdd %v4int %85 %88
OpStore %x %89
OpSelectionMerge %91 None
OpBranchConditional %84 %90 %91
%90 = OpLabel
%92 = OpLoad %v4int %x
%94 = OpIEqual %v4bool %92 %93
%95 = OpAll %bool %94
OpBranch %91
%91 = OpLabel
%96 = OpPhi %bool %false %77 %95 %90
OpStore %ok %96
%97 = OpLoad %v4int %inputGreen
%98 = OpVectorShuffle %v3int %97 %97 3 1 3
%102 = OpIMul %v3int %98 %101
%103 = OpLoad %v4int %x
%104 = OpVectorShuffle %v4int %103 %102 4 5 6 3
OpStore %x %104
OpSelectionMerge %106 None
OpBranchConditional %96 %105 %106
%105 = OpLabel
%107 = OpLoad %v4int %x
%109 = OpIEqual %v4bool %107 %108
%110 = OpAll %bool %109
OpBranch %106
%106 = OpLabel
%111 = OpPhi %bool %false %91 %110 %105
OpStore %ok %111
%112 = OpLoad %v4int %x
%113 = OpVectorShuffle %v2int %112 %112 2 3
%116 = OpSDiv %v2int %113 %115
%117 = OpLoad %v4int %x
%118 = OpVectorShuffle %v4int %117 %116 4 5 2 3
OpStore %x %118
OpSelectionMerge %120 None
OpBranchConditional %111 %119 %120
%119 = OpLabel
%121 = OpLoad %v4int %x
%123 = OpIEqual %v4bool %121 %122
%124 = OpAll %bool %123
OpBranch %120
%120 = OpLabel
%125 = OpPhi %bool %false %106 %124 %119
OpStore %ok %125
%126 = OpLoad %v4int %inputRed
%129 = OpIMul %v4int %126 %128
%130 = OpVectorShuffle %v4int %129 %129 1 0 3 2
OpStore %x %130
OpSelectionMerge %132 None
OpBranchConditional %125 %131 %132
%131 = OpLabel
%133 = OpLoad %v4int %x
%135 = OpIEqual %v4bool %133 %134
%136 = OpAll %bool %135
OpBranch %132
%132 = OpLabel
%137 = OpPhi %bool %false %120 %136 %131
OpStore %ok %137
%138 = OpLoad %v4int %inputRed
%139 = OpIAdd %v4int %61 %138
OpStore %x %139
OpSelectionMerge %141 None
OpBranchConditional %137 %140 %141
%140 = OpLabel
%142 = OpLoad %v4int %x
%143 = OpIEqual %v4bool %142 %68
%144 = OpAll %bool %143
OpBranch %141
%141 = OpLabel
%145 = OpPhi %bool %false %132 %144 %140
OpStore %ok %145
%147 = OpLoad %v4int %inputGreen
%148 = OpVectorShuffle %v4int %147 %147 1 3 0 2
%150 = OpISub %v4int %149 %148
OpStore %x %150
OpSelectionMerge %152 None
OpBranchConditional %145 %151 %152
%151 = OpLabel
%153 = OpLoad %v4int %x
%155 = OpIEqual %v4bool %153 %154
%156 = OpAll %bool %155
OpBranch %152
%152 = OpLabel
%157 = OpPhi %bool %false %141 %156 %151
OpStore %ok %157
%158 = OpLoad %v4int %inputRed
%159 = OpCompositeExtract %int %158 0
%160 = OpLoad %v4int %inputGreen
%161 = OpCompositeConstruct %v4int %159 %159 %159 %159
%162 = OpIAdd %v4int %161 %160
OpStore %x %162
OpSelectionMerge %164 None
OpBranchConditional %157 %163 %164
%163 = OpLabel
%165 = OpLoad %v4int %x
%167 = OpIEqual %v4bool %165 %166
%168 = OpAll %bool %167
OpBranch %164
%164 = OpLabel
%169 = OpPhi %bool %false %152 %168 %163
OpStore %ok %169
%170 = OpLoad %v4int %inputGreen
%171 = OpVectorShuffle %v3int %170 %170 3 1 3
%172 = OpIMul %v3int %101 %171
%173 = OpLoad %v4int %x
%174 = OpVectorShuffle %v4int %173 %172 4 5 6 3
OpStore %x %174
OpSelectionMerge %176 None
OpBranchConditional %169 %175 %176
%175 = OpLabel
%177 = OpLoad %v4int %x
%178 = OpIEqual %v4bool %177 %108
%179 = OpAll %bool %178
OpBranch %176
%176 = OpLabel
%180 = OpPhi %bool %false %164 %179 %175
OpStore %ok %180
%182 = OpLoad %v4int %x
%183 = OpVectorShuffle %v2int %182 %182 2 3
%185 = OpSDiv %v2int %184 %183
%186 = OpLoad %v4int %x
%187 = OpVectorShuffle %v4int %186 %185 4 5 2 3
OpStore %x %187
OpSelectionMerge %189 None
OpBranchConditional %180 %188 %189
%188 = OpLabel
%190 = OpLoad %v4int %x
%194 = OpIEqual %v4bool %190 %193
%195 = OpAll %bool %194
OpBranch %189
%189 = OpLabel
%196 = OpPhi %bool %false %176 %195 %188
OpStore %ok %196
%197 = OpLoad %v4int %x
%199 = OpSDiv %v4int %198 %197
%200 = OpVectorShuffle %v4int %199 %199 1 0 3 2
OpStore %x %200
OpSelectionMerge %202 None
OpBranchConditional %196 %201 %202
%201 = OpLabel
%203 = OpLoad %v4int %x
%205 = OpIEqual %v4bool %203 %204
%206 = OpAll %bool %205
OpBranch %202
%202 = OpLabel
%207 = OpPhi %bool %false %189 %206 %201
OpStore %ok %207
%208 = OpLoad %v4int %x
%209 = OpIAdd %v4int %208 %61
OpStore %x %209
%210 = OpIMul %v4int %209 %61
OpStore %x %210
%212 = OpISub %v4int %210 %211
OpStore %x %212
%213 = OpSDiv %v4int %212 %61
OpStore %x %213
OpSelectionMerge %215 None
OpBranchConditional %207 %214 %215
%214 = OpLabel
%216 = OpLoad %v4int %x
%217 = OpIEqual %v4bool %216 %204
%218 = OpAll %bool %217
OpBranch %215
%215 = OpLabel
%219 = OpPhi %bool %false %202 %218 %214
OpStore %ok %219
%220 = OpLoad %v4int %x
%221 = OpIAdd %v4int %220 %61
OpStore %x %221
%222 = OpIMul %v4int %221 %61
OpStore %x %222
%223 = OpISub %v4int %222 %211
OpStore %x %223
%224 = OpSDiv %v4int %223 %61
OpStore %x %224
OpSelectionMerge %226 None
OpBranchConditional %219 %225 %226
%225 = OpLabel
%227 = OpLoad %v4int %x
%228 = OpIEqual %v4bool %227 %204
%229 = OpAll %bool %228
OpBranch %226
%226 = OpLabel
%230 = OpPhi %bool %false %215 %229 %225
OpStore %ok %230
OpReturnValue %230
OpFunctionEnd
%main = OpFunction %v4float None %231
%232 = OpFunctionParameter %_ptr_Function_v2float
%233 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%415 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%237 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%238 = OpLoad %v4float %237
OpStore %_1_inputRed %238
%240 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%241 = OpLoad %v4float %240
OpStore %_2_inputGreen %241
%245 = OpFAdd %v4float %238 %244
OpStore %_3_x %245
OpSelectionMerge %247 None
OpBranchConditional %true %246 %247
%246 = OpLabel
%248 = OpLoad %v4float %_3_x
%251 = OpFOrdEqual %v4bool %248 %250
%252 = OpAll %bool %251
OpBranch %247
%247 = OpLabel
%253 = OpPhi %bool %false %233 %252 %246
OpStore %_0_ok %253
%254 = OpLoad %v4float %_2_inputGreen
%255 = OpVectorShuffle %v4float %254 %254 1 3 0 2
%256 = OpFSub %v4float %255 %244
OpStore %_3_x %256
OpSelectionMerge %258 None
OpBranchConditional %253 %257 %258
%257 = OpLabel
%259 = OpLoad %v4float %_3_x
%263 = OpFOrdEqual %v4bool %259 %262
%264 = OpAll %bool %263
OpBranch %258
%258 = OpLabel
%265 = OpPhi %bool %false %247 %264 %257
OpStore %_0_ok %265
%266 = OpLoad %v4float %_1_inputRed
%267 = OpLoad %v4float %_2_inputGreen
%268 = OpCompositeExtract %float %267 1
%269 = OpCompositeConstruct %v4float %268 %268 %268 %268
%270 = OpFAdd %v4float %266 %269
OpStore %_3_x %270
OpSelectionMerge %272 None
OpBranchConditional %265 %271 %272
%271 = OpLabel
%273 = OpLoad %v4float %_3_x
%276 = OpFOrdEqual %v4bool %273 %275
%277 = OpAll %bool %276
OpBranch %272
%272 = OpLabel
%278 = OpPhi %bool %false %258 %277 %271
OpStore %_0_ok %278
%279 = OpLoad %v4float %_2_inputGreen
%280 = OpVectorShuffle %v3float %279 %279 3 1 3
%283 = OpVectorTimesScalar %v3float %280 %float_9
%284 = OpLoad %v4float %_3_x
%285 = OpVectorShuffle %v4float %284 %283 4 5 6 3
OpStore %_3_x %285
OpSelectionMerge %287 None
OpBranchConditional %278 %286 %287
%286 = OpLabel
%288 = OpLoad %v4float %_3_x
%290 = OpFOrdEqual %v4bool %288 %289
%291 = OpAll %bool %290
OpBranch %287
%287 = OpLabel
%292 = OpPhi %bool %false %272 %291 %286
OpStore %_0_ok %292
%293 = OpLoad %v4float %_3_x
%294 = OpVectorShuffle %v2float %293 %293 2 3
%295 = OpVectorTimesScalar %v2float %294 %float_2
%296 = OpLoad %v4float %_3_x
%297 = OpVectorShuffle %v4float %296 %295 4 5 2 3
OpStore %_3_x %297
OpSelectionMerge %299 None
OpBranchConditional %292 %298 %299
%298 = OpLabel
%300 = OpLoad %v4float %_3_x
%304 = OpFOrdEqual %v4bool %300 %303
%305 = OpAll %bool %304
OpBranch %299
%299 = OpLabel
%306 = OpPhi %bool %false %287 %305 %298
OpStore %_0_ok %306
%307 = OpLoad %v4float %_1_inputRed
%309 = OpVectorTimesScalar %v4float %307 %float_5
%310 = OpVectorShuffle %v4float %309 %309 1 0 3 2
OpStore %_3_x %310
OpSelectionMerge %312 None
OpBranchConditional %306 %311 %312
%311 = OpLabel
%313 = OpLoad %v4float %_3_x
%315 = OpFOrdEqual %v4bool %313 %314
%316 = OpAll %bool %315
OpBranch %312
%312 = OpLabel
%317 = OpPhi %bool %false %299 %316 %311
OpStore %_0_ok %317
%318 = OpLoad %v4float %_1_inputRed
%319 = OpFAdd %v4float %244 %318
OpStore %_3_x %319
OpSelectionMerge %321 None
OpBranchConditional %317 %320 %321
%320 = OpLabel
%322 = OpLoad %v4float %_3_x
%323 = OpFOrdEqual %v4bool %322 %250
%324 = OpAll %bool %323
OpBranch %321
%321 = OpLabel
%325 = OpPhi %bool %false %312 %324 %320
OpStore %_0_ok %325
%327 = OpLoad %v4float %_2_inputGreen
%328 = OpVectorShuffle %v4float %327 %327 1 3 0 2
%330 = OpFSub %v4float %329 %328
OpStore %_3_x %330
OpSelectionMerge %332 None
OpBranchConditional %325 %331 %332
%331 = OpLabel
%333 = OpLoad %v4float %_3_x
%335 = OpFOrdEqual %v4bool %333 %334
%336 = OpAll %bool %335
OpBranch %332
%332 = OpLabel
%337 = OpPhi %bool %false %321 %336 %331
OpStore %_0_ok %337
%338 = OpLoad %v4float %_1_inputRed
%339 = OpCompositeExtract %float %338 0
%340 = OpLoad %v4float %_2_inputGreen
%341 = OpCompositeConstruct %v4float %339 %339 %339 %339
%342 = OpFAdd %v4float %341 %340
OpStore %_3_x %342
OpSelectionMerge %344 None
OpBranchConditional %337 %343 %344
%343 = OpLabel
%345 = OpLoad %v4float %_3_x
%347 = OpFOrdEqual %v4bool %345 %346
%348 = OpAll %bool %347
OpBranch %344
%344 = OpLabel
%349 = OpPhi %bool %false %332 %348 %343
OpStore %_0_ok %349
%350 = OpLoad %v4float %_2_inputGreen
%351 = OpVectorShuffle %v3float %350 %350 3 1 3
%352 = OpVectorTimesScalar %v3float %351 %float_9
%353 = OpLoad %v4float %_3_x
%354 = OpVectorShuffle %v4float %353 %352 4 5 6 3
OpStore %_3_x %354
OpSelectionMerge %356 None
OpBranchConditional %349 %355 %356
%355 = OpLabel
%357 = OpLoad %v4float %_3_x
%358 = OpFOrdEqual %v4bool %357 %289
%359 = OpAll %bool %358
OpBranch %356
%356 = OpLabel
%360 = OpPhi %bool %false %344 %359 %355
OpStore %_0_ok %360
%362 = OpLoad %v4float %_3_x
%363 = OpVectorShuffle %v2float %362 %362 2 3
%365 = OpFDiv %v2float %364 %363
%366 = OpLoad %v4float %_3_x
%367 = OpVectorShuffle %v4float %366 %365 4 5 2 3
OpStore %_3_x %367
OpSelectionMerge %369 None
OpBranchConditional %360 %368 %369
%368 = OpLabel
%370 = OpLoad %v4float %_3_x
%372 = OpFOrdEqual %v4bool %370 %371
%373 = OpAll %bool %372
OpBranch %369
%369 = OpLabel
%374 = OpPhi %bool %false %356 %373 %368
OpStore %_0_ok %374
%375 = OpLoad %v4float %_3_x
%377 = OpFDiv %v4float %376 %375
%378 = OpVectorShuffle %v4float %377 %377 1 0 3 2
OpStore %_3_x %378
OpSelectionMerge %380 None
OpBranchConditional %374 %379 %380
%379 = OpLabel
%381 = OpLoad %v4float %_3_x
%383 = OpFOrdEqual %v4bool %381 %382
%384 = OpAll %bool %383
OpBranch %380
%380 = OpLabel
%385 = OpPhi %bool %false %369 %384 %379
OpStore %_0_ok %385
%386 = OpLoad %v4float %_3_x
%387 = OpFAdd %v4float %386 %244
OpStore %_3_x %387
%388 = OpVectorTimesScalar %v4float %387 %float_2
OpStore %_3_x %388
%390 = OpFSub %v4float %388 %389
OpStore %_3_x %390
%391 = OpFDiv %float %float_1 %float_2
%392 = OpVectorTimesScalar %v4float %390 %391
OpStore %_3_x %392
OpSelectionMerge %394 None
OpBranchConditional %385 %393 %394
%393 = OpLabel
%395 = OpLoad %v4float %_3_x
%396 = OpFOrdEqual %v4bool %395 %382
%397 = OpAll %bool %396
OpBranch %394
%394 = OpLabel
%398 = OpPhi %bool %false %380 %397 %393
OpStore %_0_ok %398
%399 = OpLoad %v4float %_3_x
%400 = OpFAdd %v4float %399 %244
OpStore %_3_x %400
%401 = OpVectorTimesScalar %v4float %400 %float_2
OpStore %_3_x %401
%402 = OpFSub %v4float %401 %389
OpStore %_3_x %402
%404 = OpVectorTimesScalar %v4float %402 %float_0_5
OpStore %_3_x %404
OpSelectionMerge %406 None
OpBranchConditional %398 %405 %406
%405 = OpLabel
%407 = OpLoad %v4float %_3_x
%408 = OpFOrdEqual %v4bool %407 %382
%409 = OpAll %bool %408
OpBranch %406
%406 = OpLabel
%410 = OpPhi %bool %false %394 %409 %405
OpStore %_0_ok %410
OpSelectionMerge %412 None
OpBranchConditional %410 %411 %412
%411 = OpLabel
%413 = OpFunctionCall %bool %test_int_b
OpBranch %412
%412 = OpLabel
%414 = OpPhi %bool %false %406 %413 %411
OpSelectionMerge %418 None
OpBranchConditional %414 %416 %417
%416 = OpLabel
%419 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%420 = OpLoad %v4float %419
OpStore %415 %420
OpBranch %418
%417 = OpLabel
%421 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%422 = OpLoad %v4float %421
OpStore %415 %422
OpBranch %418
%418 = OpLabel
%423 = OpLoad %v4float %415
OpReturnValue %423
OpFunctionEnd
