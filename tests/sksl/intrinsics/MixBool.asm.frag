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
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %284 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %306 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %309 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %322 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %348 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %408 RelaxedPrecision
OpDecorate %410 RelaxedPrecision
OpDecorate %411 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %420 RelaxedPrecision
OpDecorate %421 RelaxedPrecision
OpDecorate %429 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %454 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %463 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
OpDecorate %472 RelaxedPrecision
OpDecorate %480 RelaxedPrecision
OpDecorate %490 RelaxedPrecision
OpDecorate %492 RelaxedPrecision
OpDecorate %493 RelaxedPrecision
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
%82 = OpConstantComposite %v2bool %false %false
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%102 = OpConstantComposite %v3bool %false %false %false
%v4bool = OpTypeVector %bool 4
%119 = OpConstantComposite %v4bool %false %false %false %false
%true = OpConstantTrue %bool
%151 = OpConstantComposite %v2bool %true %true
%169 = OpConstantComposite %v3bool %true %true %true
%185 = OpConstantComposite %v4bool %true %true %true %true
%int_100 = OpConstant %int 100
%202 = OpConstantComposite %v2int %int_0 %int_100
%210 = OpConstantComposite %v3int %int_0 %int_100 %int_0
%218 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
%231 = OpConstantComposite %v2int %int_100 %int_0
%239 = OpConstantComposite %v3int %int_100 %int_0 %int_0
%247 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%427 = OpConstantComposite %v2float %float_0 %float_1
%436 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%445 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%460 = OpConstantComposite %v2float %float_1 %float_0
%469 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%478 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
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
%484 = OpVariable %_ptr_Function_v4float Function
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
%61 = OpLoad %v4int %intGreen
%62 = OpCompositeExtract %int %61 0
%63 = OpLoad %v4int %intRed
%64 = OpCompositeExtract %int %63 0
%65 = OpLoad %v4int %intGreen
%66 = OpCompositeExtract %int %65 0
%67 = OpLoad %v4int %intRed
%68 = OpCompositeExtract %int %67 0
%69 = OpSelect %int %false %68 %66
%70 = OpLoad %v4int %intGreen
%71 = OpCompositeExtract %int %70 0
%72 = OpIEqual %bool %69 %71
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpLoad %v4int %intGreen
%77 = OpVectorShuffle %v2int %76 %76 0 1
%79 = OpLoad %v4int %intRed
%80 = OpVectorShuffle %v2int %79 %79 0 1
%83 = OpLoad %v4int %intGreen
%84 = OpVectorShuffle %v2int %83 %83 0 1
%85 = OpLoad %v4int %intRed
%86 = OpVectorShuffle %v2int %85 %85 0 1
%87 = OpSelect %v2int %82 %86 %84
%88 = OpLoad %v4int %intGreen
%89 = OpVectorShuffle %v2int %88 %88 0 1
%90 = OpIEqual %v2bool %87 %89
%91 = OpAll %bool %90
OpBranch %74
%74 = OpLabel
%92 = OpPhi %bool %false %25 %91 %73
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v4int %intGreen
%97 = OpVectorShuffle %v3int %96 %96 0 1 2
%99 = OpLoad %v4int %intRed
%100 = OpVectorShuffle %v3int %99 %99 0 1 2
%103 = OpLoad %v4int %intGreen
%104 = OpVectorShuffle %v3int %103 %103 0 1 2
%105 = OpLoad %v4int %intRed
%106 = OpVectorShuffle %v3int %105 %105 0 1 2
%107 = OpSelect %v3int %102 %106 %104
%108 = OpLoad %v4int %intGreen
%109 = OpVectorShuffle %v3int %108 %108 0 1 2
%110 = OpIEqual %v3bool %107 %109
%111 = OpAll %bool %110
OpBranch %94
%94 = OpLabel
%112 = OpPhi %bool %false %74 %111 %93
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpLoad %v4int %intGreen
%117 = OpLoad %v4int %intRed
%120 = OpLoad %v4int %intGreen
%121 = OpLoad %v4int %intRed
%122 = OpSelect %v4int %119 %121 %120
%123 = OpLoad %v4int %intGreen
%124 = OpIEqual %v4bool %122 %123
%125 = OpAll %bool %124
OpBranch %114
%114 = OpLabel
%126 = OpPhi %bool %false %94 %125 %113
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%130 = OpLoad %v4int %intGreen
%131 = OpCompositeExtract %int %130 0
%132 = OpLoad %v4int %intRed
%133 = OpCompositeExtract %int %132 0
%135 = OpLoad %v4int %intGreen
%136 = OpCompositeExtract %int %135 0
%137 = OpLoad %v4int %intRed
%138 = OpCompositeExtract %int %137 0
%139 = OpSelect %int %true %138 %136
%140 = OpLoad %v4int %intRed
%141 = OpCompositeExtract %int %140 0
%142 = OpIEqual %bool %139 %141
OpBranch %128
%128 = OpLabel
%143 = OpPhi %bool %false %114 %142 %127
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%147 = OpLoad %v4int %intGreen
%148 = OpVectorShuffle %v2int %147 %147 0 1
%149 = OpLoad %v4int %intRed
%150 = OpVectorShuffle %v2int %149 %149 0 1
%152 = OpLoad %v4int %intGreen
%153 = OpVectorShuffle %v2int %152 %152 0 1
%154 = OpLoad %v4int %intRed
%155 = OpVectorShuffle %v2int %154 %154 0 1
%156 = OpSelect %v2int %151 %155 %153
%157 = OpLoad %v4int %intRed
%158 = OpVectorShuffle %v2int %157 %157 0 1
%159 = OpIEqual %v2bool %156 %158
%160 = OpAll %bool %159
OpBranch %145
%145 = OpLabel
%161 = OpPhi %bool %false %128 %160 %144
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%165 = OpLoad %v4int %intGreen
%166 = OpVectorShuffle %v3int %165 %165 0 1 2
%167 = OpLoad %v4int %intRed
%168 = OpVectorShuffle %v3int %167 %167 0 1 2
%170 = OpLoad %v4int %intGreen
%171 = OpVectorShuffle %v3int %170 %170 0 1 2
%172 = OpLoad %v4int %intRed
%173 = OpVectorShuffle %v3int %172 %172 0 1 2
%174 = OpSelect %v3int %169 %173 %171
%175 = OpLoad %v4int %intRed
%176 = OpVectorShuffle %v3int %175 %175 0 1 2
%177 = OpIEqual %v3bool %174 %176
%178 = OpAll %bool %177
OpBranch %163
%163 = OpLabel
%179 = OpPhi %bool %false %145 %178 %162
OpSelectionMerge %181 None
OpBranchConditional %179 %180 %181
%180 = OpLabel
%183 = OpLoad %v4int %intGreen
%184 = OpLoad %v4int %intRed
%186 = OpLoad %v4int %intGreen
%187 = OpLoad %v4int %intRed
%188 = OpSelect %v4int %185 %187 %186
%189 = OpLoad %v4int %intRed
%190 = OpIEqual %v4bool %188 %189
%191 = OpAll %bool %190
OpBranch %181
%181 = OpLabel
%192 = OpPhi %bool %false %163 %191 %180
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpLoad %v4int %intGreen
%196 = OpCompositeExtract %int %195 0
%197 = OpIEqual %bool %int_0 %196
OpBranch %194
%194 = OpLabel
%198 = OpPhi %bool %false %181 %197 %193
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%203 = OpLoad %v4int %intGreen
%204 = OpVectorShuffle %v2int %203 %203 0 1
%205 = OpIEqual %v2bool %202 %204
%206 = OpAll %bool %205
OpBranch %200
%200 = OpLabel
%207 = OpPhi %bool %false %194 %206 %199
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
%211 = OpLoad %v4int %intGreen
%212 = OpVectorShuffle %v3int %211 %211 0 1 2
%213 = OpIEqual %v3bool %210 %212
%214 = OpAll %bool %213
OpBranch %209
%209 = OpLabel
%215 = OpPhi %bool %false %200 %214 %208
OpSelectionMerge %217 None
OpBranchConditional %215 %216 %217
%216 = OpLabel
%219 = OpLoad %v4int %intGreen
%220 = OpIEqual %v4bool %218 %219
%221 = OpAll %bool %220
OpBranch %217
%217 = OpLabel
%222 = OpPhi %bool %false %209 %221 %216
OpSelectionMerge %224 None
OpBranchConditional %222 %223 %224
%223 = OpLabel
%225 = OpLoad %v4int %intRed
%226 = OpCompositeExtract %int %225 0
%227 = OpIEqual %bool %int_100 %226
OpBranch %224
%224 = OpLabel
%228 = OpPhi %bool %false %217 %227 %223
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%232 = OpLoad %v4int %intRed
%233 = OpVectorShuffle %v2int %232 %232 0 1
%234 = OpIEqual %v2bool %231 %233
%235 = OpAll %bool %234
OpBranch %230
%230 = OpLabel
%236 = OpPhi %bool %false %224 %235 %229
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%240 = OpLoad %v4int %intRed
%241 = OpVectorShuffle %v3int %240 %240 0 1 2
%242 = OpIEqual %v3bool %239 %241
%243 = OpAll %bool %242
OpBranch %238
%238 = OpLabel
%244 = OpPhi %bool %false %230 %243 %237
OpSelectionMerge %246 None
OpBranchConditional %244 %245 %246
%245 = OpLabel
%248 = OpLoad %v4int %intRed
%249 = OpIEqual %v4bool %247 %248
%250 = OpAll %bool %249
OpBranch %246
%246 = OpLabel
%251 = OpPhi %bool %false %238 %250 %245
OpSelectionMerge %253 None
OpBranchConditional %251 %252 %253
%252 = OpLabel
%255 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%256 = OpLoad %v4float %255
%257 = OpCompositeExtract %float %256 0
%258 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%259 = OpLoad %v4float %258
%260 = OpCompositeExtract %float %259 0
%261 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%262 = OpLoad %v4float %261
%263 = OpCompositeExtract %float %262 0
%264 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%265 = OpLoad %v4float %264
%266 = OpCompositeExtract %float %265 0
%267 = OpSelect %float %false %266 %263
%268 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%269 = OpLoad %v4float %268
%270 = OpCompositeExtract %float %269 0
%271 = OpFOrdEqual %bool %267 %270
OpBranch %253
%253 = OpLabel
%272 = OpPhi %bool %false %246 %271 %252
OpSelectionMerge %274 None
OpBranchConditional %272 %273 %274
%273 = OpLabel
%276 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%277 = OpLoad %v4float %276
%278 = OpVectorShuffle %v2float %277 %277 0 1
%279 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%280 = OpLoad %v4float %279
%281 = OpVectorShuffle %v2float %280 %280 0 1
%282 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%283 = OpLoad %v4float %282
%284 = OpVectorShuffle %v2float %283 %283 0 1
%285 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%286 = OpLoad %v4float %285
%287 = OpVectorShuffle %v2float %286 %286 0 1
%288 = OpSelect %v2float %82 %287 %284
%289 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%290 = OpLoad %v4float %289
%291 = OpVectorShuffle %v2float %290 %290 0 1
%292 = OpFOrdEqual %v2bool %288 %291
%293 = OpAll %bool %292
OpBranch %274
%274 = OpLabel
%294 = OpPhi %bool %false %253 %293 %273
OpSelectionMerge %296 None
OpBranchConditional %294 %295 %296
%295 = OpLabel
%298 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%299 = OpLoad %v4float %298
%300 = OpVectorShuffle %v3float %299 %299 0 1 2
%302 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%303 = OpLoad %v4float %302
%304 = OpVectorShuffle %v3float %303 %303 0 1 2
%305 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%306 = OpLoad %v4float %305
%307 = OpVectorShuffle %v3float %306 %306 0 1 2
%308 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%309 = OpLoad %v4float %308
%310 = OpVectorShuffle %v3float %309 %309 0 1 2
%311 = OpSelect %v3float %102 %310 %307
%312 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%313 = OpLoad %v4float %312
%314 = OpVectorShuffle %v3float %313 %313 0 1 2
%315 = OpFOrdEqual %v3bool %311 %314
%316 = OpAll %bool %315
OpBranch %296
%296 = OpLabel
%317 = OpPhi %bool %false %274 %316 %295
OpSelectionMerge %319 None
OpBranchConditional %317 %318 %319
%318 = OpLabel
%321 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%322 = OpLoad %v4float %321
%323 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%324 = OpLoad %v4float %323
%325 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%326 = OpLoad %v4float %325
%327 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%328 = OpLoad %v4float %327
%329 = OpSelect %v4float %119 %328 %326
%330 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%331 = OpLoad %v4float %330
%332 = OpFOrdEqual %v4bool %329 %331
%333 = OpAll %bool %332
OpBranch %319
%319 = OpLabel
%334 = OpPhi %bool %false %296 %333 %318
OpSelectionMerge %336 None
OpBranchConditional %334 %335 %336
%335 = OpLabel
%338 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%339 = OpLoad %v4float %338
%340 = OpCompositeExtract %float %339 0
%341 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%342 = OpLoad %v4float %341
%343 = OpCompositeExtract %float %342 0
%344 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%345 = OpLoad %v4float %344
%346 = OpCompositeExtract %float %345 0
%347 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%348 = OpLoad %v4float %347
%349 = OpCompositeExtract %float %348 0
%350 = OpSelect %float %true %349 %346
%351 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%352 = OpLoad %v4float %351
%353 = OpCompositeExtract %float %352 0
%354 = OpFOrdEqual %bool %350 %353
OpBranch %336
%336 = OpLabel
%355 = OpPhi %bool %false %319 %354 %335
OpSelectionMerge %357 None
OpBranchConditional %355 %356 %357
%356 = OpLabel
%359 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%360 = OpLoad %v4float %359
%361 = OpVectorShuffle %v2float %360 %360 0 1
%362 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%363 = OpLoad %v4float %362
%364 = OpVectorShuffle %v2float %363 %363 0 1
%365 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%366 = OpLoad %v4float %365
%367 = OpVectorShuffle %v2float %366 %366 0 1
%368 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%369 = OpLoad %v4float %368
%370 = OpVectorShuffle %v2float %369 %369 0 1
%371 = OpSelect %v2float %151 %370 %367
%372 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%373 = OpLoad %v4float %372
%374 = OpVectorShuffle %v2float %373 %373 0 1
%375 = OpFOrdEqual %v2bool %371 %374
%376 = OpAll %bool %375
OpBranch %357
%357 = OpLabel
%377 = OpPhi %bool %false %336 %376 %356
OpSelectionMerge %379 None
OpBranchConditional %377 %378 %379
%378 = OpLabel
%381 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%382 = OpLoad %v4float %381
%383 = OpVectorShuffle %v3float %382 %382 0 1 2
%384 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%385 = OpLoad %v4float %384
%386 = OpVectorShuffle %v3float %385 %385 0 1 2
%387 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%388 = OpLoad %v4float %387
%389 = OpVectorShuffle %v3float %388 %388 0 1 2
%390 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%391 = OpLoad %v4float %390
%392 = OpVectorShuffle %v3float %391 %391 0 1 2
%393 = OpSelect %v3float %169 %392 %389
%394 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%395 = OpLoad %v4float %394
%396 = OpVectorShuffle %v3float %395 %395 0 1 2
%397 = OpFOrdEqual %v3bool %393 %396
%398 = OpAll %bool %397
OpBranch %379
%379 = OpLabel
%399 = OpPhi %bool %false %357 %398 %378
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%403 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%404 = OpLoad %v4float %403
%405 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%406 = OpLoad %v4float %405
%407 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%408 = OpLoad %v4float %407
%409 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%410 = OpLoad %v4float %409
%411 = OpSelect %v4float %185 %410 %408
%412 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%413 = OpLoad %v4float %412
%414 = OpFOrdEqual %v4bool %411 %413
%415 = OpAll %bool %414
OpBranch %401
%401 = OpLabel
%416 = OpPhi %bool %false %379 %415 %400
OpSelectionMerge %418 None
OpBranchConditional %416 %417 %418
%417 = OpLabel
%419 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%420 = OpLoad %v4float %419
%421 = OpCompositeExtract %float %420 0
%422 = OpFOrdEqual %bool %float_0 %421
OpBranch %418
%418 = OpLabel
%423 = OpPhi %bool %false %401 %422 %417
OpSelectionMerge %425 None
OpBranchConditional %423 %424 %425
%424 = OpLabel
%428 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%429 = OpLoad %v4float %428
%430 = OpVectorShuffle %v2float %429 %429 0 1
%431 = OpFOrdEqual %v2bool %427 %430
%432 = OpAll %bool %431
OpBranch %425
%425 = OpLabel
%433 = OpPhi %bool %false %418 %432 %424
OpSelectionMerge %435 None
OpBranchConditional %433 %434 %435
%434 = OpLabel
%437 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%438 = OpLoad %v4float %437
%439 = OpVectorShuffle %v3float %438 %438 0 1 2
%440 = OpFOrdEqual %v3bool %436 %439
%441 = OpAll %bool %440
OpBranch %435
%435 = OpLabel
%442 = OpPhi %bool %false %425 %441 %434
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%446 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%447 = OpLoad %v4float %446
%448 = OpFOrdEqual %v4bool %445 %447
%449 = OpAll %bool %448
OpBranch %444
%444 = OpLabel
%450 = OpPhi %bool %false %435 %449 %443
OpSelectionMerge %452 None
OpBranchConditional %450 %451 %452
%451 = OpLabel
%453 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%454 = OpLoad %v4float %453
%455 = OpCompositeExtract %float %454 0
%456 = OpFOrdEqual %bool %float_1 %455
OpBranch %452
%452 = OpLabel
%457 = OpPhi %bool %false %444 %456 %451
OpSelectionMerge %459 None
OpBranchConditional %457 %458 %459
%458 = OpLabel
%461 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%462 = OpLoad %v4float %461
%463 = OpVectorShuffle %v2float %462 %462 0 1
%464 = OpFOrdEqual %v2bool %460 %463
%465 = OpAll %bool %464
OpBranch %459
%459 = OpLabel
%466 = OpPhi %bool %false %452 %465 %458
OpSelectionMerge %468 None
OpBranchConditional %466 %467 %468
%467 = OpLabel
%470 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%471 = OpLoad %v4float %470
%472 = OpVectorShuffle %v3float %471 %471 0 1 2
%473 = OpFOrdEqual %v3bool %469 %472
%474 = OpAll %bool %473
OpBranch %468
%468 = OpLabel
%475 = OpPhi %bool %false %459 %474 %467
OpSelectionMerge %477 None
OpBranchConditional %475 %476 %477
%476 = OpLabel
%479 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%480 = OpLoad %v4float %479
%481 = OpFOrdEqual %v4bool %478 %480
%482 = OpAll %bool %481
OpBranch %477
%477 = OpLabel
%483 = OpPhi %bool %false %468 %482 %476
OpSelectionMerge %488 None
OpBranchConditional %483 %486 %487
%486 = OpLabel
%489 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%490 = OpLoad %v4float %489
OpStore %484 %490
OpBranch %488
%487 = OpLabel
%491 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%492 = OpLoad %v4float %491
OpStore %484 %492
OpBranch %488
%488 = OpLabel
%493 = OpLoad %v4float %484
OpReturnValue %493
OpFunctionEnd
