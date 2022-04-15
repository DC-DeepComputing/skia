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
OpDecorate %207 RelaxedPrecision
OpDecorate %_2_inputGreen RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %_3_x RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
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
%67 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%78 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
%86 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
%v3int = OpTypeVector %int 3
%int_9 = OpConstant %int 9
%93 = OpConstantComposite %v3int %int_9 %int_9 %int_9
%99 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
%v2int = OpTypeVector %int 2
%105 = OpConstantComposite %v2int %int_3 %int_3
%111 = OpConstantComposite %v4int %int_3 %int_0 %int_9 %int_2
%int_5 = OpConstant %int 5
%116 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
%121 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
%int_10 = OpConstant %int 10
%133 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
%137 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
%145 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
%int_36 = OpConstant %int 36
%160 = OpConstantComposite %v2int %int_36 %int_36
%int_4 = OpConstant %int 4
%int_18 = OpConstant %int 18
%168 = OpConstantComposite %v4int %int_4 %int_18 %int_9 %int_2
%172 = OpConstantComposite %v4int %int_36 %int_36 %int_36 %int_36
%177 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
%183 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
%200 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%213 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_3 = OpConstant %float 3
%218 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
%float_n1 = OpConstant %float -1
%float_n2 = OpConstant %float -2
%228 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
%float_1 = OpConstant %float 1
%238 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
%v3float = OpTypeVector %float 3
%float_9 = OpConstant %float 9
%250 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
%float_18 = OpConstant %float 18
%float_4 = OpConstant %float 4
%262 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
%float_5 = OpConstant %float 5
%271 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
%float_10 = OpConstant %float 10
%283 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
%287 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
%296 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
%float_36 = OpConstant %float 36
%311 = OpConstantComposite %v2float %float_36 %float_36
%317 = OpConstantComposite %v4float %float_4 %float_18 %float_9 %float_2
%321 = OpConstantComposite %v4float %float_36 %float_36 %float_36 %float_36
%326 = OpConstantComposite %v4float %float_2 %float_9 %float_18 %float_4
%332 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
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
%69 = OpIEqual %v4bool %62 %67
%70 = OpAll %bool %69
OpBranch %65
%65 = OpLabel
%71 = OpPhi %bool %false %25 %70 %64
OpStore %ok %71
%72 = OpVectorShuffle %v4int %58 %58 1 3 0 2
%73 = OpISub %v4int %72 %61
OpStore %x %73
OpSelectionMerge %75 None
OpBranchConditional %71 %74 %75
%74 = OpLabel
%79 = OpIEqual %v4bool %73 %78
%80 = OpAll %bool %79
OpBranch %75
%75 = OpLabel
%81 = OpPhi %bool %false %65 %80 %74
OpStore %ok %81
%82 = OpCompositeConstruct %v4int %53 %53 %53 %53
%83 = OpIAdd %v4int %45 %82
OpStore %x %83
OpSelectionMerge %85 None
OpBranchConditional %81 %84 %85
%84 = OpLabel
%87 = OpIEqual %v4bool %83 %86
%88 = OpAll %bool %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %75 %88 %84
OpStore %ok %89
%90 = OpVectorShuffle %v3int %58 %58 3 1 3
%94 = OpIMul %v3int %90 %93
%95 = OpLoad %v4int %x
%96 = OpVectorShuffle %v4int %95 %94 4 5 6 3
OpStore %x %96
OpSelectionMerge %98 None
OpBranchConditional %89 %97 %98
%97 = OpLabel
%100 = OpIEqual %v4bool %96 %99
%101 = OpAll %bool %100
OpBranch %98
%98 = OpLabel
%102 = OpPhi %bool %false %85 %101 %97
OpStore %ok %102
%103 = OpVectorShuffle %v2int %96 %96 2 3
%106 = OpSDiv %v2int %103 %105
%107 = OpLoad %v4int %x
%108 = OpVectorShuffle %v4int %107 %106 4 5 2 3
OpStore %x %108
OpSelectionMerge %110 None
OpBranchConditional %102 %109 %110
%109 = OpLabel
%112 = OpIEqual %v4bool %108 %111
%113 = OpAll %bool %112
OpBranch %110
%110 = OpLabel
%114 = OpPhi %bool %false %98 %113 %109
OpStore %ok %114
%117 = OpIMul %v4int %45 %116
%118 = OpVectorShuffle %v4int %117 %117 1 0 3 2
OpStore %x %118
OpSelectionMerge %120 None
OpBranchConditional %114 %119 %120
%119 = OpLabel
%122 = OpIEqual %v4bool %118 %121
%123 = OpAll %bool %122
OpBranch %120
%120 = OpLabel
%124 = OpPhi %bool %false %110 %123 %119
OpStore %ok %124
%125 = OpIAdd %v4int %61 %45
OpStore %x %125
OpSelectionMerge %127 None
OpBranchConditional %124 %126 %127
%126 = OpLabel
%128 = OpIEqual %v4bool %125 %67
%129 = OpAll %bool %128
OpBranch %127
%127 = OpLabel
%130 = OpPhi %bool %false %120 %129 %126
OpStore %ok %130
%132 = OpVectorShuffle %v4int %58 %58 1 3 0 2
%134 = OpISub %v4int %133 %132
OpStore %x %134
OpSelectionMerge %136 None
OpBranchConditional %130 %135 %136
%135 = OpLabel
%138 = OpIEqual %v4bool %134 %137
%139 = OpAll %bool %138
OpBranch %136
%136 = OpLabel
%140 = OpPhi %bool %false %127 %139 %135
OpStore %ok %140
%141 = OpCompositeConstruct %v4int %38 %38 %38 %38
%142 = OpIAdd %v4int %141 %58
OpStore %x %142
OpSelectionMerge %144 None
OpBranchConditional %140 %143 %144
%143 = OpLabel
%146 = OpIEqual %v4bool %142 %145
%147 = OpAll %bool %146
OpBranch %144
%144 = OpLabel
%148 = OpPhi %bool %false %136 %147 %143
OpStore %ok %148
%149 = OpVectorShuffle %v3int %58 %58 3 1 3
%150 = OpIMul %v3int %93 %149
%151 = OpLoad %v4int %x
%152 = OpVectorShuffle %v4int %151 %150 4 5 6 3
OpStore %x %152
OpSelectionMerge %154 None
OpBranchConditional %148 %153 %154
%153 = OpLabel
%155 = OpIEqual %v4bool %152 %99
%156 = OpAll %bool %155
OpBranch %154
%154 = OpLabel
%157 = OpPhi %bool %false %144 %156 %153
OpStore %ok %157
%159 = OpVectorShuffle %v2int %152 %152 2 3
%161 = OpSDiv %v2int %160 %159
%162 = OpLoad %v4int %x
%163 = OpVectorShuffle %v4int %162 %161 4 5 2 3
OpStore %x %163
OpSelectionMerge %165 None
OpBranchConditional %157 %164 %165
%164 = OpLabel
%169 = OpIEqual %v4bool %163 %168
%170 = OpAll %bool %169
OpBranch %165
%165 = OpLabel
%171 = OpPhi %bool %false %154 %170 %164
OpStore %ok %171
%173 = OpSDiv %v4int %172 %163
%174 = OpVectorShuffle %v4int %173 %173 1 0 3 2
OpStore %x %174
OpSelectionMerge %176 None
OpBranchConditional %171 %175 %176
%175 = OpLabel
%178 = OpIEqual %v4bool %174 %177
%179 = OpAll %bool %178
OpBranch %176
%176 = OpLabel
%180 = OpPhi %bool %false %165 %179 %175
OpStore %ok %180
%181 = OpIAdd %v4int %174 %61
OpStore %x %181
%182 = OpIMul %v4int %181 %61
OpStore %x %182
%184 = OpISub %v4int %182 %183
OpStore %x %184
%185 = OpSDiv %v4int %184 %61
OpStore %x %185
OpSelectionMerge %187 None
OpBranchConditional %180 %186 %187
%186 = OpLabel
%188 = OpIEqual %v4bool %185 %177
%189 = OpAll %bool %188
OpBranch %187
%187 = OpLabel
%190 = OpPhi %bool %false %176 %189 %186
OpStore %ok %190
%191 = OpIAdd %v4int %185 %61
OpStore %x %191
%192 = OpIMul %v4int %191 %61
OpStore %x %192
%193 = OpISub %v4int %192 %183
OpStore %x %193
%194 = OpSDiv %v4int %193 %61
OpStore %x %194
OpSelectionMerge %196 None
OpBranchConditional %190 %195 %196
%195 = OpLabel
%197 = OpIEqual %v4bool %194 %177
%198 = OpAll %bool %197
OpBranch %196
%196 = OpLabel
%199 = OpPhi %bool %false %187 %198 %195
OpStore %ok %199
OpReturnValue %199
OpFunctionEnd
%main = OpFunction %v4float None %200
%201 = OpFunctionParameter %_ptr_Function_v2float
%202 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_inputRed = OpVariable %_ptr_Function_v4float Function
%_2_inputGreen = OpVariable %_ptr_Function_v4float Function
%_3_x = OpVariable %_ptr_Function_v4float Function
%355 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%206 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%207 = OpLoad %v4float %206
OpStore %_1_inputRed %207
%209 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%210 = OpLoad %v4float %209
OpStore %_2_inputGreen %210
%214 = OpFAdd %v4float %207 %213
OpStore %_3_x %214
OpSelectionMerge %216 None
OpBranchConditional %true %215 %216
%215 = OpLabel
%219 = OpFOrdEqual %v4bool %214 %218
%220 = OpAll %bool %219
OpBranch %216
%216 = OpLabel
%221 = OpPhi %bool %false %202 %220 %215
OpStore %_0_ok %221
%222 = OpVectorShuffle %v4float %210 %210 1 3 0 2
%223 = OpFSub %v4float %222 %213
OpStore %_3_x %223
OpSelectionMerge %225 None
OpBranchConditional %221 %224 %225
%224 = OpLabel
%229 = OpFOrdEqual %v4bool %223 %228
%230 = OpAll %bool %229
OpBranch %225
%225 = OpLabel
%231 = OpPhi %bool %false %216 %230 %224
OpStore %_0_ok %231
%232 = OpCompositeExtract %float %210 1
%233 = OpCompositeConstruct %v4float %232 %232 %232 %232
%234 = OpFAdd %v4float %207 %233
OpStore %_3_x %234
OpSelectionMerge %236 None
OpBranchConditional %231 %235 %236
%235 = OpLabel
%239 = OpFOrdEqual %v4bool %234 %238
%240 = OpAll %bool %239
OpBranch %236
%236 = OpLabel
%241 = OpPhi %bool %false %225 %240 %235
OpStore %_0_ok %241
%242 = OpVectorShuffle %v3float %210 %210 3 1 3
%245 = OpVectorTimesScalar %v3float %242 %float_9
%246 = OpLoad %v4float %_3_x
%247 = OpVectorShuffle %v4float %246 %245 4 5 6 3
OpStore %_3_x %247
OpSelectionMerge %249 None
OpBranchConditional %241 %248 %249
%248 = OpLabel
%251 = OpFOrdEqual %v4bool %247 %250
%252 = OpAll %bool %251
OpBranch %249
%249 = OpLabel
%253 = OpPhi %bool %false %236 %252 %248
OpStore %_0_ok %253
%254 = OpVectorShuffle %v2float %247 %247 2 3
%255 = OpVectorTimesScalar %v2float %254 %float_2
%256 = OpLoad %v4float %_3_x
%257 = OpVectorShuffle %v4float %256 %255 4 5 2 3
OpStore %_3_x %257
OpSelectionMerge %259 None
OpBranchConditional %253 %258 %259
%258 = OpLabel
%263 = OpFOrdEqual %v4bool %257 %262
%264 = OpAll %bool %263
OpBranch %259
%259 = OpLabel
%265 = OpPhi %bool %false %249 %264 %258
OpStore %_0_ok %265
%267 = OpVectorTimesScalar %v4float %207 %float_5
%268 = OpVectorShuffle %v4float %267 %267 1 0 3 2
OpStore %_3_x %268
OpSelectionMerge %270 None
OpBranchConditional %265 %269 %270
%269 = OpLabel
%272 = OpFOrdEqual %v4bool %268 %271
%273 = OpAll %bool %272
OpBranch %270
%270 = OpLabel
%274 = OpPhi %bool %false %259 %273 %269
OpStore %_0_ok %274
%275 = OpFAdd %v4float %213 %207
OpStore %_3_x %275
OpSelectionMerge %277 None
OpBranchConditional %274 %276 %277
%276 = OpLabel
%278 = OpFOrdEqual %v4bool %275 %218
%279 = OpAll %bool %278
OpBranch %277
%277 = OpLabel
%280 = OpPhi %bool %false %270 %279 %276
OpStore %_0_ok %280
%282 = OpVectorShuffle %v4float %210 %210 1 3 0 2
%284 = OpFSub %v4float %283 %282
OpStore %_3_x %284
OpSelectionMerge %286 None
OpBranchConditional %280 %285 %286
%285 = OpLabel
%288 = OpFOrdEqual %v4bool %284 %287
%289 = OpAll %bool %288
OpBranch %286
%286 = OpLabel
%290 = OpPhi %bool %false %277 %289 %285
OpStore %_0_ok %290
%291 = OpCompositeExtract %float %207 0
%292 = OpCompositeConstruct %v4float %291 %291 %291 %291
%293 = OpFAdd %v4float %292 %210
OpStore %_3_x %293
OpSelectionMerge %295 None
OpBranchConditional %290 %294 %295
%294 = OpLabel
%297 = OpFOrdEqual %v4bool %293 %296
%298 = OpAll %bool %297
OpBranch %295
%295 = OpLabel
%299 = OpPhi %bool %false %286 %298 %294
OpStore %_0_ok %299
%300 = OpVectorShuffle %v3float %210 %210 3 1 3
%301 = OpVectorTimesScalar %v3float %300 %float_9
%302 = OpLoad %v4float %_3_x
%303 = OpVectorShuffle %v4float %302 %301 4 5 6 3
OpStore %_3_x %303
OpSelectionMerge %305 None
OpBranchConditional %299 %304 %305
%304 = OpLabel
%306 = OpFOrdEqual %v4bool %303 %250
%307 = OpAll %bool %306
OpBranch %305
%305 = OpLabel
%308 = OpPhi %bool %false %295 %307 %304
OpStore %_0_ok %308
%310 = OpVectorShuffle %v2float %303 %303 2 3
%312 = OpFDiv %v2float %311 %310
%313 = OpLoad %v4float %_3_x
%314 = OpVectorShuffle %v4float %313 %312 4 5 2 3
OpStore %_3_x %314
OpSelectionMerge %316 None
OpBranchConditional %308 %315 %316
%315 = OpLabel
%318 = OpFOrdEqual %v4bool %314 %317
%319 = OpAll %bool %318
OpBranch %316
%316 = OpLabel
%320 = OpPhi %bool %false %305 %319 %315
OpStore %_0_ok %320
%322 = OpFDiv %v4float %321 %314
%323 = OpVectorShuffle %v4float %322 %322 1 0 3 2
OpStore %_3_x %323
OpSelectionMerge %325 None
OpBranchConditional %320 %324 %325
%324 = OpLabel
%327 = OpFOrdEqual %v4bool %323 %326
%328 = OpAll %bool %327
OpBranch %325
%325 = OpLabel
%329 = OpPhi %bool %false %316 %328 %324
OpStore %_0_ok %329
%330 = OpFAdd %v4float %323 %213
OpStore %_3_x %330
%331 = OpVectorTimesScalar %v4float %330 %float_2
OpStore %_3_x %331
%333 = OpFSub %v4float %331 %332
OpStore %_3_x %333
%334 = OpFDiv %float %float_1 %float_2
%335 = OpVectorTimesScalar %v4float %333 %334
OpStore %_3_x %335
OpSelectionMerge %337 None
OpBranchConditional %329 %336 %337
%336 = OpLabel
%338 = OpFOrdEqual %v4bool %335 %326
%339 = OpAll %bool %338
OpBranch %337
%337 = OpLabel
%340 = OpPhi %bool %false %325 %339 %336
OpStore %_0_ok %340
%341 = OpFAdd %v4float %335 %213
OpStore %_3_x %341
%342 = OpVectorTimesScalar %v4float %341 %float_2
OpStore %_3_x %342
%343 = OpFSub %v4float %342 %332
OpStore %_3_x %343
%345 = OpVectorTimesScalar %v4float %343 %float_0_5
OpStore %_3_x %345
OpSelectionMerge %347 None
OpBranchConditional %340 %346 %347
%346 = OpLabel
%348 = OpFOrdEqual %v4bool %345 %326
%349 = OpAll %bool %348
OpBranch %347
%347 = OpLabel
%350 = OpPhi %bool %false %337 %349 %346
OpStore %_0_ok %350
OpSelectionMerge %352 None
OpBranchConditional %350 %351 %352
%351 = OpLabel
%353 = OpFunctionCall %bool %test_int_b
OpBranch %352
%352 = OpLabel
%354 = OpPhi %bool %false %347 %353 %351
OpSelectionMerge %358 None
OpBranchConditional %354 %356 %357
%356 = OpLabel
%359 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%360 = OpLoad %v4float %359
OpStore %355 %360
OpBranch %358
%357 = OpLabel
%361 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%362 = OpLoad %v4float %361
OpStore %355 %362
OpBranch %358
%358 = OpLabel
%363 = OpLoad %v4float %355
OpReturnValue %363
OpFunctionEnd
