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
OpDecorate %65 RelaxedPrecision
OpDecorate %_1_inputRed RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
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
%62 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
%false = OpConstantFalse %bool
%int_3 = OpConstant %int 3
%70 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%82 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
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
%133 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%147 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
%151 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%162 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%180 = OpConstantComposite %v2int %int_36 %int_36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%189 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%194 = OpConstantComposite %v4int %int_36 %int_36 %int_36 %int_36
%199 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%206 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%224 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%238 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_3 = OpConstant %float 3
%245 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%256 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%268 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%282 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%296 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%306 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%320 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%324 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%335 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%353 = OpConstantComposite %v2float %float_36 %float_36
%360 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%365 = OpConstantComposite %v4float %float_36 %float_36 %float_36 %float_36
%370 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%377 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
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
%60 = OpLoad %v4int %inputRed
%63 = OpIAdd %v4int %60 %62
OpStore %x %63
%65 = OpLoad %bool %ok
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%68 = OpLoad %v4int %x
%71 = OpIEqual %v4bool %68 %70
%73 = OpAll %bool %71
OpBranch %67
%67 = OpLabel
%74 = OpPhi %bool %false %25 %73 %66
OpStore %ok %74
%75 = OpLoad %v4int %inputGreen
%76 = OpVectorShuffle %v4int %75 %75 1 3 0 2
%77 = OpISub %v4int %76 %62
OpStore %x %77
OpSelectionMerge %79 None
OpBranchConditional %74 %78 %79
%78 = OpLabel
%83 = OpIEqual %v4bool %77 %82
%84 = OpAll %bool %83
OpBranch %79
%79 = OpLabel
%85 = OpPhi %bool %false %67 %84 %78
OpStore %ok %85
%86 = OpLoad %v4int %inputRed
%87 = OpLoad %v4int %inputGreen
%88 = OpCompositeExtract %int %87 1
%89 = OpCompositeConstruct %v4int %88 %88 %88 %88
%90 = OpIAdd %v4int %86 %89
OpStore %x %90
OpSelectionMerge %92 None
OpBranchConditional %85 %91 %92
%91 = OpLabel
%94 = OpIEqual %v4bool %90 %93
%95 = OpAll %bool %94
OpBranch %92
%92 = OpLabel
%96 = OpPhi %bool %false %79 %95 %91
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
%111 = OpPhi %bool %false %92 %110 %105
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
%134 = OpIEqual %v4bool %130 %133
%135 = OpAll %bool %134
OpBranch %132
%132 = OpLabel
%136 = OpPhi %bool %false %120 %135 %131
OpStore %ok %136
%137 = OpLoad %v4int %inputRed
%138 = OpIAdd %v4int %62 %137
OpStore %x %138
OpSelectionMerge %140 None
OpBranchConditional %136 %139 %140
%139 = OpLabel
%141 = OpIEqual %v4bool %138 %70
%142 = OpAll %bool %141
OpBranch %140
%140 = OpLabel
%143 = OpPhi %bool %false %132 %142 %139
OpStore %ok %143
%145 = OpLoad %v4int %inputGreen
%146 = OpVectorShuffle %v4int %145 %145 1 3 0 2
%148 = OpISub %v4int %147 %146
OpStore %x %148
OpSelectionMerge %150 None
OpBranchConditional %143 %149 %150
%149 = OpLabel
%152 = OpIEqual %v4bool %148 %151
%153 = OpAll %bool %152
OpBranch %150
%150 = OpLabel
%154 = OpPhi %bool %false %140 %153 %149
OpStore %ok %154
%155 = OpLoad %v4int %inputRed
%156 = OpCompositeExtract %int %155 0
%157 = OpLoad %v4int %inputGreen
%158 = OpCompositeConstruct %v4int %156 %156 %156 %156
%159 = OpIAdd %v4int %158 %157
OpStore %x %159
OpSelectionMerge %161 None
OpBranchConditional %154 %160 %161
%160 = OpLabel
%163 = OpIEqual %v4bool %159 %162
%164 = OpAll %bool %163
OpBranch %161
%161 = OpLabel
%165 = OpPhi %bool %false %150 %164 %160
OpStore %ok %165
%166 = OpLoad %v4int %inputGreen
%167 = OpVectorShuffle %v3int %166 %166 3 1 3
%168 = OpIMul %v3int %101 %167
%169 = OpLoad %v4int %x
%170 = OpVectorShuffle %v4int %169 %168 4 5 6 3
OpStore %x %170
OpSelectionMerge %172 None
OpBranchConditional %165 %171 %172
%171 = OpLabel
%173 = OpLoad %v4int %x
%174 = OpIEqual %v4bool %173 %108
%175 = OpAll %bool %174
OpBranch %172
%172 = OpLabel
%176 = OpPhi %bool %false %161 %175 %171
OpStore %ok %176
%178 = OpLoad %v4int %x
%179 = OpVectorShuffle %v2int %178 %178 2 3
%181 = OpSDiv %v2int %180 %179
%182 = OpLoad %v4int %x
%183 = OpVectorShuffle %v4int %182 %181 4 5 2 3
OpStore %x %183
OpSelectionMerge %185 None
OpBranchConditional %176 %184 %185
%184 = OpLabel
%186 = OpLoad %v4int %x
%190 = OpIEqual %v4bool %186 %189
%191 = OpAll %bool %190
OpBranch %185
%185 = OpLabel
%192 = OpPhi %bool %false %172 %191 %184
OpStore %ok %192
%193 = OpLoad %v4int %x
%195 = OpSDiv %v4int %194 %193
%196 = OpVectorShuffle %v4int %195 %195 1 0 3 2
OpStore %x %196
OpSelectionMerge %198 None
OpBranchConditional %192 %197 %198
%197 = OpLabel
%200 = OpIEqual %v4bool %196 %199
%201 = OpAll %bool %200
OpBranch %198
%198 = OpLabel
%202 = OpPhi %bool %false %185 %201 %197
OpStore %ok %202
%203 = OpLoad %v4int %x
%204 = OpIAdd %v4int %203 %62
OpStore %x %204
%205 = OpIMul %v4int %204 %62
OpStore %x %205
%207 = OpISub %v4int %205 %206
OpStore %x %207
%208 = OpSDiv %v4int %207 %62
OpStore %x %208
OpSelectionMerge %210 None
OpBranchConditional %202 %209 %210
%209 = OpLabel
%211 = OpIEqual %v4bool %208 %199
%212 = OpAll %bool %211
OpBranch %210
%210 = OpLabel
%213 = OpPhi %bool %false %198 %212 %209
OpStore %ok %213
%214 = OpLoad %v4int %x
%215 = OpIAdd %v4int %214 %62
OpStore %x %215
%216 = OpIMul %v4int %215 %62
OpStore %x %216
%217 = OpISub %v4int %216 %206
OpStore %x %217
%218 = OpSDiv %v4int %217 %62
OpStore %x %218
OpSelectionMerge %220 None
OpBranchConditional %213 %219 %220
%219 = OpLabel
%221 = OpIEqual %v4bool %218 %199
%222 = OpAll %bool %221
OpBranch %220
%220 = OpLabel
%223 = OpPhi %bool %false %210 %222 %219
OpStore %ok %223
OpReturnValue %223
OpFunctionEnd
%main = OpFunction %v4float None %224
%225 = OpFunctionParameter %_ptr_Function_v2float
%226 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%401 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%230 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%231 = OpLoad %v4float %230
OpStore %_1_inputRed %231
%233 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%234 = OpLoad %v4float %233
OpStore %_2_inputGreen %234
%236 = OpLoad %v4float %_1_inputRed
%239 = OpFAdd %v4float %236 %238
OpStore %_3_x %239
%240 = OpLoad %bool %_0_ok
OpSelectionMerge %242 None
OpBranchConditional %240 %241 %242
%241 = OpLabel
%243 = OpLoad %v4float %_3_x
%246 = OpFOrdEqual %v4bool %243 %245
%247 = OpAll %bool %246
OpBranch %242
%242 = OpLabel
%248 = OpPhi %bool %false %226 %247 %241
OpStore %_0_ok %248
%249 = OpLoad %v4float %_2_inputGreen
%250 = OpVectorShuffle %v4float %249 %249 1 3 0 2
%251 = OpFSub %v4float %250 %238
OpStore %_3_x %251
OpSelectionMerge %253 None
OpBranchConditional %248 %252 %253
%252 = OpLabel
%257 = OpFOrdEqual %v4bool %251 %256
%258 = OpAll %bool %257
OpBranch %253
%253 = OpLabel
%259 = OpPhi %bool %false %242 %258 %252
OpStore %_0_ok %259
%260 = OpLoad %v4float %_1_inputRed
%261 = OpLoad %v4float %_2_inputGreen
%262 = OpCompositeExtract %float %261 1
%263 = OpCompositeConstruct %v4float %262 %262 %262 %262
%264 = OpFAdd %v4float %260 %263
OpStore %_3_x %264
OpSelectionMerge %266 None
OpBranchConditional %259 %265 %266
%265 = OpLabel
%269 = OpFOrdEqual %v4bool %264 %268
%270 = OpAll %bool %269
OpBranch %266
%266 = OpLabel
%271 = OpPhi %bool %false %253 %270 %265
OpStore %_0_ok %271
%272 = OpLoad %v4float %_2_inputGreen
%273 = OpVectorShuffle %v3float %272 %272 3 1 3
%276 = OpVectorTimesScalar %v3float %273 %float_9
%277 = OpLoad %v4float %_3_x
%278 = OpVectorShuffle %v4float %277 %276 4 5 6 3
OpStore %_3_x %278
OpSelectionMerge %280 None
OpBranchConditional %271 %279 %280
%279 = OpLabel
%281 = OpLoad %v4float %_3_x
%283 = OpFOrdEqual %v4bool %281 %282
%284 = OpAll %bool %283
OpBranch %280
%280 = OpLabel
%285 = OpPhi %bool %false %266 %284 %279
OpStore %_0_ok %285
%286 = OpLoad %v4float %_3_x
%287 = OpVectorShuffle %v2float %286 %286 2 3
%288 = OpVectorTimesScalar %v2float %287 %float_2
%289 = OpLoad %v4float %_3_x
%290 = OpVectorShuffle %v4float %289 %288 4 5 2 3
OpStore %_3_x %290
OpSelectionMerge %292 None
OpBranchConditional %285 %291 %292
%291 = OpLabel
%293 = OpLoad %v4float %_3_x
%297 = OpFOrdEqual %v4bool %293 %296
%298 = OpAll %bool %297
OpBranch %292
%292 = OpLabel
%299 = OpPhi %bool %false %280 %298 %291
OpStore %_0_ok %299
%300 = OpLoad %v4float %_1_inputRed
%302 = OpVectorTimesScalar %v4float %300 %float_5
%303 = OpVectorShuffle %v4float %302 %302 1 0 3 2
OpStore %_3_x %303
OpSelectionMerge %305 None
OpBranchConditional %299 %304 %305
%304 = OpLabel
%307 = OpFOrdEqual %v4bool %303 %306
%308 = OpAll %bool %307
OpBranch %305
%305 = OpLabel
%309 = OpPhi %bool %false %292 %308 %304
OpStore %_0_ok %309
%310 = OpLoad %v4float %_1_inputRed
%311 = OpFAdd %v4float %238 %310
OpStore %_3_x %311
OpSelectionMerge %313 None
OpBranchConditional %309 %312 %313
%312 = OpLabel
%314 = OpFOrdEqual %v4bool %311 %245
%315 = OpAll %bool %314
OpBranch %313
%313 = OpLabel
%316 = OpPhi %bool %false %305 %315 %312
OpStore %_0_ok %316
%318 = OpLoad %v4float %_2_inputGreen
%319 = OpVectorShuffle %v4float %318 %318 1 3 0 2
%321 = OpFSub %v4float %320 %319
OpStore %_3_x %321
OpSelectionMerge %323 None
OpBranchConditional %316 %322 %323
%322 = OpLabel
%325 = OpFOrdEqual %v4bool %321 %324
%326 = OpAll %bool %325
OpBranch %323
%323 = OpLabel
%327 = OpPhi %bool %false %313 %326 %322
OpStore %_0_ok %327
%328 = OpLoad %v4float %_1_inputRed
%329 = OpCompositeExtract %float %328 0
%330 = OpLoad %v4float %_2_inputGreen
%331 = OpCompositeConstruct %v4float %329 %329 %329 %329
%332 = OpFAdd %v4float %331 %330
OpStore %_3_x %332
OpSelectionMerge %334 None
OpBranchConditional %327 %333 %334
%333 = OpLabel
%336 = OpFOrdEqual %v4bool %332 %335
%337 = OpAll %bool %336
OpBranch %334
%334 = OpLabel
%338 = OpPhi %bool %false %323 %337 %333
OpStore %_0_ok %338
%339 = OpLoad %v4float %_2_inputGreen
%340 = OpVectorShuffle %v3float %339 %339 3 1 3
%341 = OpVectorTimesScalar %v3float %340 %float_9
%342 = OpLoad %v4float %_3_x
%343 = OpVectorShuffle %v4float %342 %341 4 5 6 3
OpStore %_3_x %343
OpSelectionMerge %345 None
OpBranchConditional %338 %344 %345
%344 = OpLabel
%346 = OpLoad %v4float %_3_x
%347 = OpFOrdEqual %v4bool %346 %282
%348 = OpAll %bool %347
OpBranch %345
%345 = OpLabel
%349 = OpPhi %bool %false %334 %348 %344
OpStore %_0_ok %349
%351 = OpLoad %v4float %_3_x
%352 = OpVectorShuffle %v2float %351 %351 2 3
%354 = OpFDiv %v2float %353 %352
%355 = OpLoad %v4float %_3_x
%356 = OpVectorShuffle %v4float %355 %354 4 5 2 3
OpStore %_3_x %356
OpSelectionMerge %358 None
OpBranchConditional %349 %357 %358
%357 = OpLabel
%359 = OpLoad %v4float %_3_x
%361 = OpFOrdEqual %v4bool %359 %360
%362 = OpAll %bool %361
OpBranch %358
%358 = OpLabel
%363 = OpPhi %bool %false %345 %362 %357
OpStore %_0_ok %363
%364 = OpLoad %v4float %_3_x
%366 = OpFDiv %v4float %365 %364
%367 = OpVectorShuffle %v4float %366 %366 1 0 3 2
OpStore %_3_x %367
OpSelectionMerge %369 None
OpBranchConditional %363 %368 %369
%368 = OpLabel
%371 = OpFOrdEqual %v4bool %367 %370
%372 = OpAll %bool %371
OpBranch %369
%369 = OpLabel
%373 = OpPhi %bool %false %358 %372 %368
OpStore %_0_ok %373
%374 = OpLoad %v4float %_3_x
%375 = OpFAdd %v4float %374 %238
OpStore %_3_x %375
%376 = OpVectorTimesScalar %v4float %375 %float_2
OpStore %_3_x %376
%378 = OpFSub %v4float %376 %377
OpStore %_3_x %378
%379 = OpFDiv %float %float_1 %float_2
%380 = OpVectorTimesScalar %v4float %378 %379
OpStore %_3_x %380
OpSelectionMerge %382 None
OpBranchConditional %373 %381 %382
%381 = OpLabel
%383 = OpFOrdEqual %v4bool %380 %370
%384 = OpAll %bool %383
OpBranch %382
%382 = OpLabel
%385 = OpPhi %bool %false %369 %384 %381
OpStore %_0_ok %385
%386 = OpLoad %v4float %_3_x
%387 = OpFAdd %v4float %386 %238
OpStore %_3_x %387
%388 = OpVectorTimesScalar %v4float %387 %float_2
OpStore %_3_x %388
%389 = OpFSub %v4float %388 %377
OpStore %_3_x %389
%391 = OpVectorTimesScalar %v4float %389 %float_0_5
OpStore %_3_x %391
OpSelectionMerge %393 None
OpBranchConditional %385 %392 %393
%392 = OpLabel
%394 = OpFOrdEqual %v4bool %391 %370
%395 = OpAll %bool %394
OpBranch %393
%393 = OpLabel
%396 = OpPhi %bool %false %382 %395 %392
OpStore %_0_ok %396
OpSelectionMerge %398 None
OpBranchConditional %396 %397 %398
%397 = OpLabel
%399 = OpFunctionCall %bool %test_int_b
OpBranch %398
%398 = OpLabel
%400 = OpPhi %bool %false %393 %399 %397
OpSelectionMerge %404 None
OpBranchConditional %400 %402 %403
%402 = OpLabel
%405 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%406 = OpLoad %v4float %405
OpStore %401 %406
OpBranch %404
%403 = OpLabel
%407 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%408 = OpLoad %v4float %407
OpStore %401 %408
OpBranch %404
%404 = OpLabel
%409 = OpLoad %v4float %401
OpReturnValue %409
OpFunctionEnd
