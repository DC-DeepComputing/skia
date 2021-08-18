OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
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
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %398 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %401 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %418 RelaxedPrecision
OpDecorate %419 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %436 RelaxedPrecision
OpDecorate %437 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %443 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %459 RelaxedPrecision
OpDecorate %460 RelaxedPrecision
OpDecorate %461 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %467 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %481 RelaxedPrecision
OpDecorate %483 RelaxedPrecision
OpDecorate %484 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%int_3 = OpConstant %int 3
%72 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%87 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%100 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%116 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%131 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%144 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%167 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%180 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%210 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%222 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%265 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%286 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%300 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%314 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%329 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%344 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%356 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%379 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%392 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%419 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%431 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%float_0_5 = OpConstant %float 0.5
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%test_int_b = OpFunction %bool None %26
%27 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%inputRed = OpVariable %_ptr_Function_v4int Function
%inputGreen = OpVariable %_ptr_Function_v4int Function
%x = OpVariable %_ptr_Function_v4int Function
OpStore %ok %true
%35 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%38 = OpLoad %v4float %35
%39 = OpCompositeExtract %float %38 0
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %38 1
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %38 2
%44 = OpConvertFToS %int %43
%45 = OpCompositeExtract %float %38 3
%46 = OpConvertFToS %int %45
%47 = OpCompositeConstruct %v4int %40 %42 %44 %46
OpStore %inputRed %47
%49 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%51 = OpLoad %v4float %49
%52 = OpCompositeExtract %float %51 0
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %51 1
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %51 2
%57 = OpConvertFToS %int %56
%58 = OpCompositeExtract %float %51 3
%59 = OpConvertFToS %int %58
%60 = OpCompositeConstruct %v4int %53 %55 %57 %59
OpStore %inputGreen %60
%62 = OpLoad %v4int %inputRed
%64 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%65 = OpIAdd %v4int %62 %64
OpStore %x %65
%67 = OpLoad %bool %ok
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %v4int %x
%73 = OpIEqual %v4bool %70 %72
%75 = OpAll %bool %73
OpBranch %69
%69 = OpLabel
%76 = OpPhi %bool %false %27 %75 %68
OpStore %ok %76
%77 = OpLoad %v4int %inputGreen
%78 = OpVectorShuffle %v4int %77 %77 1 3 0 2
%79 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%80 = OpISub %v4int %78 %79
OpStore %x %80
%81 = OpLoad %bool %ok
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpLoad %v4int %x
%88 = OpIEqual %v4bool %84 %87
%89 = OpAll %bool %88
OpBranch %83
%83 = OpLabel
%90 = OpPhi %bool %false %69 %89 %82
OpStore %ok %90
%91 = OpLoad %v4int %inputRed
%92 = OpLoad %v4int %inputGreen
%93 = OpCompositeExtract %int %92 1
%94 = OpCompositeConstruct %v4int %93 %93 %93 %93
%95 = OpIAdd %v4int %91 %94
OpStore %x %95
%96 = OpLoad %bool %ok
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %v4int %x
%101 = OpIEqual %v4bool %99 %100
%102 = OpAll %bool %101
OpBranch %98
%98 = OpLabel
%103 = OpPhi %bool %false %83 %102 %97
OpStore %ok %103
%104 = OpLoad %v4int %inputGreen
%105 = OpVectorShuffle %v3int %104 %104 3 1 3
%108 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%109 = OpIMul %v3int %105 %108
%110 = OpLoad %v4int %x
%111 = OpVectorShuffle %v4int %110 %109 4 5 6 3
OpStore %x %111
%112 = OpLoad %bool %ok
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpLoad %v4int %x
%117 = OpIEqual %v4bool %115 %116
%118 = OpAll %bool %117
OpBranch %114
%114 = OpLabel
%119 = OpPhi %bool %false %98 %118 %113
OpStore %ok %119
%120 = OpLoad %v4int %x
%121 = OpVectorShuffle %v2int %120 %120 2 3
%123 = OpCompositeConstruct %v2int %int_3 %int_3
%124 = OpSDiv %v2int %121 %123
%125 = OpLoad %v4int %x
%126 = OpVectorShuffle %v4int %125 %124 4 5 2 3
OpStore %x %126
%127 = OpLoad %bool %ok
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpLoad %v4int %x
%132 = OpIEqual %v4bool %130 %131
%133 = OpAll %bool %132
OpBranch %129
%129 = OpLabel
%134 = OpPhi %bool %false %114 %133 %128
OpStore %ok %134
%135 = OpLoad %v4int %inputRed
%137 = OpCompositeConstruct %v4int %int_5 %int_5 %int_5 %int_5
%138 = OpIMul %v4int %135 %137
%139 = OpVectorShuffle %v4int %138 %138 1 0 3 2
OpStore %x %139
%140 = OpLoad %bool %ok
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpLoad %v4int %x
%145 = OpIEqual %v4bool %143 %144
%146 = OpAll %bool %145
OpBranch %142
%142 = OpLabel
%147 = OpPhi %bool %false %129 %146 %141
OpStore %ok %147
%148 = OpLoad %v4int %inputRed
%149 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%150 = OpIAdd %v4int %149 %148
OpStore %x %150
%151 = OpLoad %bool %ok
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpLoad %v4int %x
%155 = OpIEqual %v4bool %154 %72
%156 = OpAll %bool %155
OpBranch %153
%153 = OpLabel
%157 = OpPhi %bool %false %142 %156 %152
OpStore %ok %157
%159 = OpLoad %v4int %inputGreen
%160 = OpVectorShuffle %v4int %159 %159 1 3 0 2
%161 = OpCompositeConstruct %v4int %int_10 %int_10 %int_10 %int_10
%162 = OpISub %v4int %161 %160
OpStore %x %162
%163 = OpLoad %bool %ok
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%166 = OpLoad %v4int %x
%168 = OpIEqual %v4bool %166 %167
%169 = OpAll %bool %168
OpBranch %165
%165 = OpLabel
%170 = OpPhi %bool %false %153 %169 %164
OpStore %ok %170
%171 = OpLoad %v4int %inputRed
%172 = OpCompositeExtract %int %171 0
%173 = OpLoad %v4int %inputGreen
%174 = OpCompositeConstruct %v4int %172 %172 %172 %172
%175 = OpIAdd %v4int %174 %173
OpStore %x %175
%176 = OpLoad %bool %ok
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpLoad %v4int %x
%181 = OpIEqual %v4bool %179 %180
%182 = OpAll %bool %181
OpBranch %178
%178 = OpLabel
%183 = OpPhi %bool %false %165 %182 %177
OpStore %ok %183
%184 = OpLoad %v4int %inputGreen
%185 = OpVectorShuffle %v3int %184 %184 3 1 3
%186 = OpCompositeConstruct %v3int %int_9 %int_9 %int_9
%187 = OpIMul %v3int %186 %185
%188 = OpLoad %v4int %x
%189 = OpVectorShuffle %v4int %188 %187 4 5 6 3
OpStore %x %189
%190 = OpLoad %bool %ok
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%193 = OpLoad %v4int %x
%194 = OpIEqual %v4bool %193 %116
%195 = OpAll %bool %194
OpBranch %192
%192 = OpLabel
%196 = OpPhi %bool %false %178 %195 %191
OpStore %ok %196
%198 = OpLoad %v4int %x
%199 = OpVectorShuffle %v2int %198 %198 2 3
%200 = OpCompositeConstruct %v2int %int_36 %int_36
%201 = OpSDiv %v2int %200 %199
%202 = OpLoad %v4int %x
%203 = OpVectorShuffle %v4int %202 %201 4 5 2 3
OpStore %x %203
%204 = OpLoad %bool %ok
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%207 = OpLoad %v4int %x
%211 = OpIEqual %v4bool %207 %210
%212 = OpAll %bool %211
OpBranch %206
%206 = OpLabel
%213 = OpPhi %bool %false %192 %212 %205
OpStore %ok %213
%214 = OpLoad %v4int %x
%215 = OpCompositeConstruct %v4int %int_36 %int_36 %int_36 %int_36
%216 = OpSDiv %v4int %215 %214
%217 = OpVectorShuffle %v4int %216 %216 1 0 3 2
OpStore %x %217
%218 = OpLoad %bool %ok
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpLoad %v4int %x
%223 = OpIEqual %v4bool %221 %222
%224 = OpAll %bool %223
OpBranch %220
%220 = OpLabel
%225 = OpPhi %bool %false %206 %224 %219
OpStore %ok %225
%226 = OpLoad %v4int %x
%227 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%228 = OpIAdd %v4int %226 %227
OpStore %x %228
%229 = OpLoad %v4int %x
%230 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%231 = OpIMul %v4int %229 %230
OpStore %x %231
%232 = OpLoad %v4int %x
%233 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%234 = OpISub %v4int %232 %233
OpStore %x %234
%235 = OpLoad %v4int %x
%236 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%237 = OpSDiv %v4int %235 %236
OpStore %x %237
%238 = OpLoad %bool %ok
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%241 = OpLoad %v4int %x
%242 = OpIEqual %v4bool %241 %222
%243 = OpAll %bool %242
OpBranch %240
%240 = OpLabel
%244 = OpPhi %bool %false %220 %243 %239
OpStore %ok %244
%245 = OpLoad %v4int %x
%246 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%247 = OpIAdd %v4int %245 %246
OpStore %x %247
%248 = OpLoad %v4int %x
%249 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%250 = OpIMul %v4int %248 %249
OpStore %x %250
%251 = OpLoad %v4int %x
%252 = OpCompositeConstruct %v4int %int_4 %int_4 %int_4 %int_4
%253 = OpISub %v4int %251 %252
OpStore %x %253
%254 = OpLoad %v4int %x
%255 = OpCompositeConstruct %v4int %int_2 %int_2 %int_2 %int_2
%256 = OpSDiv %v4int %254 %255
OpStore %x %256
%257 = OpLoad %bool %ok
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%260 = OpLoad %v4int %x
%261 = OpIEqual %v4bool %260 %222
%262 = OpAll %bool %261
OpBranch %259
%259 = OpLabel
%263 = OpPhi %bool %false %240 %262 %258
OpStore %ok %263
%264 = OpLoad %bool %ok
OpReturnValue %264
OpFunctionEnd
%main = OpFunction %v4float None %265
%266 = OpFunctionParameter %_ptr_Function_v2float
%267 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%476 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%271 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%272 = OpLoad %v4float %271
OpStore %_1_inputRed %272
%274 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%275 = OpLoad %v4float %274
OpStore %_2_inputGreen %275
%277 = OpLoad %v4float %_1_inputRed
%279 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%280 = OpFAdd %v4float %277 %279
OpStore %_3_x %280
%281 = OpLoad %bool %_0_ok
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpLoad %v4float %_3_x
%287 = OpFOrdEqual %v4bool %284 %286
%288 = OpAll %bool %287
OpBranch %283
%283 = OpLabel
%289 = OpPhi %bool %false %267 %288 %282
OpStore %_0_ok %289
%290 = OpLoad %v4float %_2_inputGreen
%291 = OpVectorShuffle %v4float %290 %290 1 3 0 2
%292 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%293 = OpFSub %v4float %291 %292
OpStore %_3_x %293
%294 = OpLoad %bool %_0_ok
OpSelectionMerge %296 None
OpBranchConditional %294 %295 %296
%295 = OpLabel
%297 = OpLoad %v4float %_3_x
%301 = OpFOrdEqual %v4bool %297 %300
%302 = OpAll %bool %301
OpBranch %296
%296 = OpLabel
%303 = OpPhi %bool %false %283 %302 %295
OpStore %_0_ok %303
%304 = OpLoad %v4float %_1_inputRed
%305 = OpLoad %v4float %_2_inputGreen
%306 = OpCompositeExtract %float %305 1
%307 = OpCompositeConstruct %v4float %306 %306 %306 %306
%308 = OpFAdd %v4float %304 %307
OpStore %_3_x %308
%309 = OpLoad %bool %_0_ok
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpLoad %v4float %_3_x
%315 = OpFOrdEqual %v4bool %312 %314
%316 = OpAll %bool %315
OpBranch %311
%311 = OpLabel
%317 = OpPhi %bool %false %296 %316 %310
OpStore %_0_ok %317
%318 = OpLoad %v4float %_2_inputGreen
%319 = OpVectorShuffle %v3float %318 %318 3 1 3
%322 = OpVectorTimesScalar %v3float %319 %float_9
%323 = OpLoad %v4float %_3_x
%324 = OpVectorShuffle %v4float %323 %322 4 5 6 3
OpStore %_3_x %324
%325 = OpLoad %bool %_0_ok
OpSelectionMerge %327 None
OpBranchConditional %325 %326 %327
%326 = OpLabel
%328 = OpLoad %v4float %_3_x
%330 = OpFOrdEqual %v4bool %328 %329
%331 = OpAll %bool %330
OpBranch %327
%327 = OpLabel
%332 = OpPhi %bool %false %311 %331 %326
OpStore %_0_ok %332
%333 = OpLoad %v4float %_3_x
%334 = OpVectorShuffle %v2float %333 %333 2 3
%335 = OpVectorTimesScalar %v2float %334 %float_2
%336 = OpLoad %v4float %_3_x
%337 = OpVectorShuffle %v4float %336 %335 4 5 2 3
OpStore %_3_x %337
%338 = OpLoad %bool %_0_ok
OpSelectionMerge %340 None
OpBranchConditional %338 %339 %340
%339 = OpLabel
%341 = OpLoad %v4float %_3_x
%345 = OpFOrdEqual %v4bool %341 %344
%346 = OpAll %bool %345
OpBranch %340
%340 = OpLabel
%347 = OpPhi %bool %false %327 %346 %339
OpStore %_0_ok %347
%348 = OpLoad %v4float %_1_inputRed
%350 = OpVectorTimesScalar %v4float %348 %float_5
%351 = OpVectorShuffle %v4float %350 %350 1 0 3 2
OpStore %_3_x %351
%352 = OpLoad %bool %_0_ok
OpSelectionMerge %354 None
OpBranchConditional %352 %353 %354
%353 = OpLabel
%355 = OpLoad %v4float %_3_x
%357 = OpFOrdEqual %v4bool %355 %356
%358 = OpAll %bool %357
OpBranch %354
%354 = OpLabel
%359 = OpPhi %bool %false %340 %358 %353
OpStore %_0_ok %359
%360 = OpLoad %v4float %_1_inputRed
%361 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%362 = OpFAdd %v4float %361 %360
OpStore %_3_x %362
%363 = OpLoad %bool %_0_ok
OpSelectionMerge %365 None
OpBranchConditional %363 %364 %365
%364 = OpLabel
%366 = OpLoad %v4float %_3_x
%367 = OpFOrdEqual %v4bool %366 %286
%368 = OpAll %bool %367
OpBranch %365
%365 = OpLabel
%369 = OpPhi %bool %false %354 %368 %364
OpStore %_0_ok %369
%371 = OpLoad %v4float %_2_inputGreen
%372 = OpVectorShuffle %v4float %371 %371 1 3 0 2
%373 = OpCompositeConstruct %v4float %float_10 %float_10 %float_10 %float_10
%374 = OpFSub %v4float %373 %372
OpStore %_3_x %374
%375 = OpLoad %bool %_0_ok
OpSelectionMerge %377 None
OpBranchConditional %375 %376 %377
%376 = OpLabel
%378 = OpLoad %v4float %_3_x
%380 = OpFOrdEqual %v4bool %378 %379
%381 = OpAll %bool %380
OpBranch %377
%377 = OpLabel
%382 = OpPhi %bool %false %365 %381 %376
OpStore %_0_ok %382
%383 = OpLoad %v4float %_1_inputRed
%384 = OpCompositeExtract %float %383 0
%385 = OpLoad %v4float %_2_inputGreen
%386 = OpCompositeConstruct %v4float %384 %384 %384 %384
%387 = OpFAdd %v4float %386 %385
OpStore %_3_x %387
%388 = OpLoad %bool %_0_ok
OpSelectionMerge %390 None
OpBranchConditional %388 %389 %390
%389 = OpLabel
%391 = OpLoad %v4float %_3_x
%393 = OpFOrdEqual %v4bool %391 %392
%394 = OpAll %bool %393
OpBranch %390
%390 = OpLabel
%395 = OpPhi %bool %false %377 %394 %389
OpStore %_0_ok %395
%396 = OpLoad %v4float %_2_inputGreen
%397 = OpVectorShuffle %v3float %396 %396 3 1 3
%398 = OpVectorTimesScalar %v3float %397 %float_9
%399 = OpLoad %v4float %_3_x
%400 = OpVectorShuffle %v4float %399 %398 4 5 6 3
OpStore %_3_x %400
%401 = OpLoad %bool %_0_ok
OpSelectionMerge %403 None
OpBranchConditional %401 %402 %403
%402 = OpLabel
%404 = OpLoad %v4float %_3_x
%405 = OpFOrdEqual %v4bool %404 %329
%406 = OpAll %bool %405
OpBranch %403
%403 = OpLabel
%407 = OpPhi %bool %false %390 %406 %402
OpStore %_0_ok %407
%409 = OpLoad %v4float %_3_x
%410 = OpVectorShuffle %v2float %409 %409 2 3
%411 = OpCompositeConstruct %v2float %float_36 %float_36
%412 = OpFDiv %v2float %411 %410
%413 = OpLoad %v4float %_3_x
%414 = OpVectorShuffle %v4float %413 %412 4 5 2 3
OpStore %_3_x %414
%415 = OpLoad %bool %_0_ok
OpSelectionMerge %417 None
OpBranchConditional %415 %416 %417
%416 = OpLabel
%418 = OpLoad %v4float %_3_x
%420 = OpFOrdEqual %v4bool %418 %419
%421 = OpAll %bool %420
OpBranch %417
%417 = OpLabel
%422 = OpPhi %bool %false %403 %421 %416
OpStore %_0_ok %422
%423 = OpLoad %v4float %_3_x
%424 = OpCompositeConstruct %v4float %float_36 %float_36 %float_36 %float_36
%425 = OpFDiv %v4float %424 %423
%426 = OpVectorShuffle %v4float %425 %425 1 0 3 2
OpStore %_3_x %426
%427 = OpLoad %bool %_0_ok
OpSelectionMerge %429 None
OpBranchConditional %427 %428 %429
%428 = OpLabel
%430 = OpLoad %v4float %_3_x
%432 = OpFOrdEqual %v4bool %430 %431
%433 = OpAll %bool %432
OpBranch %429
%429 = OpLabel
%434 = OpPhi %bool %false %417 %433 %428
OpStore %_0_ok %434
%435 = OpLoad %v4float %_3_x
%436 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%437 = OpFAdd %v4float %435 %436
OpStore %_3_x %437
%438 = OpLoad %v4float %_3_x
%439 = OpVectorTimesScalar %v4float %438 %float_2
OpStore %_3_x %439
%440 = OpLoad %v4float %_3_x
%441 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%442 = OpFSub %v4float %440 %441
OpStore %_3_x %442
%443 = OpLoad %v4float %_3_x
%444 = OpFDiv %float %float_1 %float_2
%445 = OpVectorTimesScalar %v4float %443 %444
OpStore %_3_x %445
%446 = OpLoad %bool %_0_ok
OpSelectionMerge %448 None
OpBranchConditional %446 %447 %448
%447 = OpLabel
%449 = OpLoad %v4float %_3_x
%450 = OpFOrdEqual %v4bool %449 %431
%451 = OpAll %bool %450
OpBranch %448
%448 = OpLabel
%452 = OpPhi %bool %false %429 %451 %447
OpStore %_0_ok %452
%453 = OpLoad %v4float %_3_x
%454 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%455 = OpFAdd %v4float %453 %454
OpStore %_3_x %455
%456 = OpLoad %v4float %_3_x
%457 = OpVectorTimesScalar %v4float %456 %float_2
OpStore %_3_x %457
%458 = OpLoad %v4float %_3_x
%459 = OpCompositeConstruct %v4float %float_4 %float_4 %float_4 %float_4
%460 = OpFSub %v4float %458 %459
OpStore %_3_x %460
%461 = OpLoad %v4float %_3_x
%463 = OpVectorTimesScalar %v4float %461 %float_0_5
OpStore %_3_x %463
%464 = OpLoad %bool %_0_ok
OpSelectionMerge %466 None
OpBranchConditional %464 %465 %466
%465 = OpLabel
%467 = OpLoad %v4float %_3_x
%468 = OpFOrdEqual %v4bool %467 %431
%469 = OpAll %bool %468
OpBranch %466
%466 = OpLabel
%470 = OpPhi %bool %false %448 %469 %465
OpStore %_0_ok %470
%471 = OpLoad %bool %_0_ok
OpSelectionMerge %473 None
OpBranchConditional %471 %472 %473
%472 = OpLabel
%474 = OpFunctionCall %bool %test_int_b
OpBranch %473
%473 = OpLabel
%475 = OpPhi %bool %false %466 %474 %472
OpSelectionMerge %479 None
OpBranchConditional %475 %477 %478
%477 = OpLabel
%480 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%481 = OpLoad %v4float %480
OpStore %476 %481
OpBranch %479
%478 = OpLabel
%482 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%483 = OpLoad %v4float %482
OpStore %476 %483
OpBranch %479
%479 = OpLabel
%484 = OpLoad %v4float %476
OpReturnValue %484
OpFunctionEnd
