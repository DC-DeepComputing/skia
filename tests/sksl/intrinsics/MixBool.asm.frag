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
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %intGreen "intGreen"
OpName %intRed "intRed"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %285 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %294 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %325 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %351 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %469 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%73 = OpConstantComposite %v2bool %false %false
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%92 = OpConstantComposite %v3bool %false %false %false
%v4bool = OpTypeVector %bool 4
%108 = OpConstantComposite %v4bool %false %false %false %false
%true = OpConstantTrue %bool
%138 = OpConstantComposite %v2bool %true %true
%155 = OpConstantComposite %v3bool %true %true %true
%170 = OpConstantComposite %v4bool %true %true %true %true
%int_100 = OpConstant %int 100
%186 = OpConstantComposite %v2int %int_0 %int_100
%194 = OpConstantComposite %v3int %int_0 %int_100 %int_0
%202 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
%215 = OpConstantComposite %v2int %int_100 %int_0
%223 = OpConstantComposite %v3int %int_100 %int_0 %int_0
%231 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%403 = OpConstantComposite %v2float %float_0 %float_1
%412 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%421 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%436 = OpConstantComposite %v2float %float_1 %float_0
%445 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%454 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%intGreen = OpVariable %_ptr_Function_v4int Function
%intRed = OpVariable %_ptr_Function_v4int Function
%460 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %30
%35 = OpVectorTimesScalar %v4float %33 %float_100
%36 = OpCompositeExtract %float %35 0
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %35 1
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %35 2
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %35 3
%43 = OpConvertFToS %int %42
%44 = OpCompositeConstruct %v4int %37 %39 %41 %43
OpStore %intGreen %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%48 = OpLoad %v4float %46
%49 = OpVectorTimesScalar %v4float %48 %float_100
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %intRed %58
%61 = OpCompositeExtract %int %44 0
%62 = OpCompositeExtract %int %58 0
%60 = OpSelect %int %false %62 %61
%63 = OpIEqual %bool %60 %61
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpLoad %v4int %intGreen
%68 = OpVectorShuffle %v2int %67 %67 0 1
%70 = OpLoad %v4int %intRed
%71 = OpVectorShuffle %v2int %70 %70 0 1
%74 = OpLoad %v4int %intGreen
%75 = OpVectorShuffle %v2int %74 %74 0 1
%76 = OpLoad %v4int %intRed
%77 = OpVectorShuffle %v2int %76 %76 0 1
%66 = OpSelect %v2int %73 %77 %75
%78 = OpLoad %v4int %intGreen
%79 = OpVectorShuffle %v2int %78 %78 0 1
%80 = OpIEqual %v2bool %66 %79
%81 = OpAll %bool %80
OpBranch %65
%65 = OpLabel
%82 = OpPhi %bool %false %25 %81 %64
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpLoad %v4int %intGreen
%87 = OpVectorShuffle %v3int %86 %86 0 1 2
%89 = OpLoad %v4int %intRed
%90 = OpVectorShuffle %v3int %89 %89 0 1 2
%93 = OpLoad %v4int %intGreen
%94 = OpVectorShuffle %v3int %93 %93 0 1 2
%95 = OpLoad %v4int %intRed
%96 = OpVectorShuffle %v3int %95 %95 0 1 2
%85 = OpSelect %v3int %92 %96 %94
%97 = OpLoad %v4int %intGreen
%98 = OpVectorShuffle %v3int %97 %97 0 1 2
%99 = OpIEqual %v3bool %85 %98
%100 = OpAll %bool %99
OpBranch %84
%84 = OpLabel
%101 = OpPhi %bool %false %65 %100 %83
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%105 = OpLoad %v4int %intGreen
%106 = OpLoad %v4int %intRed
%109 = OpLoad %v4int %intGreen
%110 = OpLoad %v4int %intRed
%104 = OpSelect %v4int %108 %110 %109
%111 = OpLoad %v4int %intGreen
%112 = OpIEqual %v4bool %104 %111
%113 = OpAll %bool %112
OpBranch %103
%103 = OpLabel
%114 = OpPhi %bool %false %84 %113 %102
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %v4int %intGreen
%119 = OpCompositeExtract %int %118 0
%120 = OpLoad %v4int %intRed
%121 = OpCompositeExtract %int %120 0
%123 = OpLoad %v4int %intGreen
%124 = OpCompositeExtract %int %123 0
%125 = OpLoad %v4int %intRed
%126 = OpCompositeExtract %int %125 0
%117 = OpSelect %int %true %126 %124
%127 = OpLoad %v4int %intRed
%128 = OpCompositeExtract %int %127 0
%129 = OpIEqual %bool %117 %128
OpBranch %116
%116 = OpLabel
%130 = OpPhi %bool %false %103 %129 %115
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%134 = OpLoad %v4int %intGreen
%135 = OpVectorShuffle %v2int %134 %134 0 1
%136 = OpLoad %v4int %intRed
%137 = OpVectorShuffle %v2int %136 %136 0 1
%139 = OpLoad %v4int %intGreen
%140 = OpVectorShuffle %v2int %139 %139 0 1
%141 = OpLoad %v4int %intRed
%142 = OpVectorShuffle %v2int %141 %141 0 1
%133 = OpSelect %v2int %138 %142 %140
%143 = OpLoad %v4int %intRed
%144 = OpVectorShuffle %v2int %143 %143 0 1
%145 = OpIEqual %v2bool %133 %144
%146 = OpAll %bool %145
OpBranch %132
%132 = OpLabel
%147 = OpPhi %bool %false %116 %146 %131
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpLoad %v4int %intGreen
%152 = OpVectorShuffle %v3int %151 %151 0 1 2
%153 = OpLoad %v4int %intRed
%154 = OpVectorShuffle %v3int %153 %153 0 1 2
%156 = OpLoad %v4int %intGreen
%157 = OpVectorShuffle %v3int %156 %156 0 1 2
%158 = OpLoad %v4int %intRed
%159 = OpVectorShuffle %v3int %158 %158 0 1 2
%150 = OpSelect %v3int %155 %159 %157
%160 = OpLoad %v4int %intRed
%161 = OpVectorShuffle %v3int %160 %160 0 1 2
%162 = OpIEqual %v3bool %150 %161
%163 = OpAll %bool %162
OpBranch %149
%149 = OpLabel
%164 = OpPhi %bool %false %132 %163 %148
OpSelectionMerge %166 None
OpBranchConditional %164 %165 %166
%165 = OpLabel
%168 = OpLoad %v4int %intGreen
%169 = OpLoad %v4int %intRed
%171 = OpLoad %v4int %intGreen
%172 = OpLoad %v4int %intRed
%167 = OpSelect %v4int %170 %172 %171
%173 = OpLoad %v4int %intRed
%174 = OpIEqual %v4bool %167 %173
%175 = OpAll %bool %174
OpBranch %166
%166 = OpLabel
%176 = OpPhi %bool %false %149 %175 %165
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpLoad %v4int %intGreen
%180 = OpCompositeExtract %int %179 0
%181 = OpIEqual %bool %int_0 %180
OpBranch %178
%178 = OpLabel
%182 = OpPhi %bool %false %166 %181 %177
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%187 = OpLoad %v4int %intGreen
%188 = OpVectorShuffle %v2int %187 %187 0 1
%189 = OpIEqual %v2bool %186 %188
%190 = OpAll %bool %189
OpBranch %184
%184 = OpLabel
%191 = OpPhi %bool %false %178 %190 %183
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%195 = OpLoad %v4int %intGreen
%196 = OpVectorShuffle %v3int %195 %195 0 1 2
%197 = OpIEqual %v3bool %194 %196
%198 = OpAll %bool %197
OpBranch %193
%193 = OpLabel
%199 = OpPhi %bool %false %184 %198 %192
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%203 = OpLoad %v4int %intGreen
%204 = OpIEqual %v4bool %202 %203
%205 = OpAll %bool %204
OpBranch %201
%201 = OpLabel
%206 = OpPhi %bool %false %193 %205 %200
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpLoad %v4int %intRed
%210 = OpCompositeExtract %int %209 0
%211 = OpIEqual %bool %int_100 %210
OpBranch %208
%208 = OpLabel
%212 = OpPhi %bool %false %201 %211 %207
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%216 = OpLoad %v4int %intRed
%217 = OpVectorShuffle %v2int %216 %216 0 1
%218 = OpIEqual %v2bool %215 %217
%219 = OpAll %bool %218
OpBranch %214
%214 = OpLabel
%220 = OpPhi %bool %false %208 %219 %213
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%224 = OpLoad %v4int %intRed
%225 = OpVectorShuffle %v3int %224 %224 0 1 2
%226 = OpIEqual %v3bool %223 %225
%227 = OpAll %bool %226
OpBranch %222
%222 = OpLabel
%228 = OpPhi %bool %false %214 %227 %221
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%232 = OpLoad %v4int %intRed
%233 = OpIEqual %v4bool %231 %232
%234 = OpAll %bool %233
OpBranch %230
%230 = OpLabel
%235 = OpPhi %bool %false %222 %234 %229
OpSelectionMerge %237 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
%239 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%240 = OpLoad %v4float %239
%241 = OpCompositeExtract %float %240 0
%242 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%243 = OpLoad %v4float %242
%244 = OpCompositeExtract %float %243 0
%245 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%246 = OpLoad %v4float %245
%247 = OpCompositeExtract %float %246 0
%248 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%249 = OpLoad %v4float %248
%250 = OpCompositeExtract %float %249 0
%238 = OpSelect %float %false %250 %247
%251 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%252 = OpLoad %v4float %251
%253 = OpCompositeExtract %float %252 0
%254 = OpFOrdEqual %bool %238 %253
OpBranch %237
%237 = OpLabel
%255 = OpPhi %bool %false %230 %254 %236
OpSelectionMerge %257 None
OpBranchConditional %255 %256 %257
%256 = OpLabel
%259 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%260 = OpLoad %v4float %259
%261 = OpVectorShuffle %v2float %260 %260 0 1
%262 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%263 = OpLoad %v4float %262
%264 = OpVectorShuffle %v2float %263 %263 0 1
%265 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%266 = OpLoad %v4float %265
%267 = OpVectorShuffle %v2float %266 %266 0 1
%268 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%269 = OpLoad %v4float %268
%270 = OpVectorShuffle %v2float %269 %269 0 1
%258 = OpSelect %v2float %73 %270 %267
%271 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%272 = OpLoad %v4float %271
%273 = OpVectorShuffle %v2float %272 %272 0 1
%274 = OpFOrdEqual %v2bool %258 %273
%275 = OpAll %bool %274
OpBranch %257
%257 = OpLabel
%276 = OpPhi %bool %false %237 %275 %256
OpSelectionMerge %278 None
OpBranchConditional %276 %277 %278
%277 = OpLabel
%280 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%281 = OpLoad %v4float %280
%282 = OpVectorShuffle %v3float %281 %281 0 1 2
%284 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%285 = OpLoad %v4float %284
%286 = OpVectorShuffle %v3float %285 %285 0 1 2
%287 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%288 = OpLoad %v4float %287
%289 = OpVectorShuffle %v3float %288 %288 0 1 2
%290 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%291 = OpLoad %v4float %290
%292 = OpVectorShuffle %v3float %291 %291 0 1 2
%279 = OpSelect %v3float %92 %292 %289
%293 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%294 = OpLoad %v4float %293
%295 = OpVectorShuffle %v3float %294 %294 0 1 2
%296 = OpFOrdEqual %v3bool %279 %295
%297 = OpAll %bool %296
OpBranch %278
%278 = OpLabel
%298 = OpPhi %bool %false %257 %297 %277
OpSelectionMerge %300 None
OpBranchConditional %298 %299 %300
%299 = OpLabel
%302 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%303 = OpLoad %v4float %302
%304 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%305 = OpLoad %v4float %304
%306 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%307 = OpLoad %v4float %306
%308 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%309 = OpLoad %v4float %308
%301 = OpSelect %v4float %108 %309 %307
%310 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%311 = OpLoad %v4float %310
%312 = OpFOrdEqual %v4bool %301 %311
%313 = OpAll %bool %312
OpBranch %300
%300 = OpLabel
%314 = OpPhi %bool %false %278 %313 %299
OpSelectionMerge %316 None
OpBranchConditional %314 %315 %316
%315 = OpLabel
%318 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%319 = OpLoad %v4float %318
%320 = OpCompositeExtract %float %319 0
%321 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%322 = OpLoad %v4float %321
%323 = OpCompositeExtract %float %322 0
%324 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%325 = OpLoad %v4float %324
%326 = OpCompositeExtract %float %325 0
%327 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%328 = OpLoad %v4float %327
%329 = OpCompositeExtract %float %328 0
%317 = OpSelect %float %true %329 %326
%330 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%331 = OpLoad %v4float %330
%332 = OpCompositeExtract %float %331 0
%333 = OpFOrdEqual %bool %317 %332
OpBranch %316
%316 = OpLabel
%334 = OpPhi %bool %false %300 %333 %315
OpSelectionMerge %336 None
OpBranchConditional %334 %335 %336
%335 = OpLabel
%338 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%339 = OpLoad %v4float %338
%340 = OpVectorShuffle %v2float %339 %339 0 1
%341 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%342 = OpLoad %v4float %341
%343 = OpVectorShuffle %v2float %342 %342 0 1
%344 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%345 = OpLoad %v4float %344
%346 = OpVectorShuffle %v2float %345 %345 0 1
%347 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%348 = OpLoad %v4float %347
%349 = OpVectorShuffle %v2float %348 %348 0 1
%337 = OpSelect %v2float %138 %349 %346
%350 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%351 = OpLoad %v4float %350
%352 = OpVectorShuffle %v2float %351 %351 0 1
%353 = OpFOrdEqual %v2bool %337 %352
%354 = OpAll %bool %353
OpBranch %336
%336 = OpLabel
%355 = OpPhi %bool %false %316 %354 %335
OpSelectionMerge %357 None
OpBranchConditional %355 %356 %357
%356 = OpLabel
%359 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%360 = OpLoad %v4float %359
%361 = OpVectorShuffle %v3float %360 %360 0 1 2
%362 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%363 = OpLoad %v4float %362
%364 = OpVectorShuffle %v3float %363 %363 0 1 2
%365 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%366 = OpLoad %v4float %365
%367 = OpVectorShuffle %v3float %366 %366 0 1 2
%368 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%369 = OpLoad %v4float %368
%370 = OpVectorShuffle %v3float %369 %369 0 1 2
%358 = OpSelect %v3float %155 %370 %367
%371 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%372 = OpLoad %v4float %371
%373 = OpVectorShuffle %v3float %372 %372 0 1 2
%374 = OpFOrdEqual %v3bool %358 %373
%375 = OpAll %bool %374
OpBranch %357
%357 = OpLabel
%376 = OpPhi %bool %false %336 %375 %356
OpSelectionMerge %378 None
OpBranchConditional %376 %377 %378
%377 = OpLabel
%380 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%381 = OpLoad %v4float %380
%382 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%383 = OpLoad %v4float %382
%384 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%385 = OpLoad %v4float %384
%386 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%387 = OpLoad %v4float %386
%379 = OpSelect %v4float %170 %387 %385
%388 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%389 = OpLoad %v4float %388
%390 = OpFOrdEqual %v4bool %379 %389
%391 = OpAll %bool %390
OpBranch %378
%378 = OpLabel
%392 = OpPhi %bool %false %357 %391 %377
OpSelectionMerge %394 None
OpBranchConditional %392 %393 %394
%393 = OpLabel
%395 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%396 = OpLoad %v4float %395
%397 = OpCompositeExtract %float %396 0
%398 = OpFOrdEqual %bool %float_0 %397
OpBranch %394
%394 = OpLabel
%399 = OpPhi %bool %false %378 %398 %393
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%404 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%405 = OpLoad %v4float %404
%406 = OpVectorShuffle %v2float %405 %405 0 1
%407 = OpFOrdEqual %v2bool %403 %406
%408 = OpAll %bool %407
OpBranch %401
%401 = OpLabel
%409 = OpPhi %bool %false %394 %408 %400
OpSelectionMerge %411 None
OpBranchConditional %409 %410 %411
%410 = OpLabel
%413 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%414 = OpLoad %v4float %413
%415 = OpVectorShuffle %v3float %414 %414 0 1 2
%416 = OpFOrdEqual %v3bool %412 %415
%417 = OpAll %bool %416
OpBranch %411
%411 = OpLabel
%418 = OpPhi %bool %false %401 %417 %410
OpSelectionMerge %420 None
OpBranchConditional %418 %419 %420
%419 = OpLabel
%422 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%423 = OpLoad %v4float %422
%424 = OpFOrdEqual %v4bool %421 %423
%425 = OpAll %bool %424
OpBranch %420
%420 = OpLabel
%426 = OpPhi %bool %false %411 %425 %419
OpSelectionMerge %428 None
OpBranchConditional %426 %427 %428
%427 = OpLabel
%429 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%430 = OpLoad %v4float %429
%431 = OpCompositeExtract %float %430 0
%432 = OpFOrdEqual %bool %float_1 %431
OpBranch %428
%428 = OpLabel
%433 = OpPhi %bool %false %420 %432 %427
OpSelectionMerge %435 None
OpBranchConditional %433 %434 %435
%434 = OpLabel
%437 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%438 = OpLoad %v4float %437
%439 = OpVectorShuffle %v2float %438 %438 0 1
%440 = OpFOrdEqual %v2bool %436 %439
%441 = OpAll %bool %440
OpBranch %435
%435 = OpLabel
%442 = OpPhi %bool %false %428 %441 %434
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%446 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%447 = OpLoad %v4float %446
%448 = OpVectorShuffle %v3float %447 %447 0 1 2
%449 = OpFOrdEqual %v3bool %445 %448
%450 = OpAll %bool %449
OpBranch %444
%444 = OpLabel
%451 = OpPhi %bool %false %435 %450 %443
OpSelectionMerge %453 None
OpBranchConditional %451 %452 %453
%452 = OpLabel
%455 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%456 = OpLoad %v4float %455
%457 = OpFOrdEqual %v4bool %454 %456
%458 = OpAll %bool %457
OpBranch %453
%453 = OpLabel
%459 = OpPhi %bool %false %444 %458 %452
OpSelectionMerge %464 None
OpBranchConditional %459 %462 %463
%462 = OpLabel
%465 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%466 = OpLoad %v4float %465
OpStore %460 %466
OpBranch %464
%463 = OpLabel
%467 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%468 = OpLoad %v4float %467
OpStore %460 %468
OpBranch %464
%464 = OpLabel
%469 = OpLoad %v4float %460
OpReturnValue %469
OpFunctionEnd
