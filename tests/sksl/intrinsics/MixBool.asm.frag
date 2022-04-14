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
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %278 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %281 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %312 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %340 RelaxedPrecision
OpDecorate %346 RelaxedPrecision
OpDecorate %347 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %357 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %368 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %390 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %394 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %405 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %423 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %438 RelaxedPrecision
OpDecorate %439 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %474 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %477 RelaxedPrecision
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
%80 = OpConstantComposite %v2bool %false %false
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%99 = OpConstantComposite %v3bool %false %false %false
%v4bool = OpTypeVector %bool 4
%115 = OpConstantComposite %v4bool %false %false %false %false
%true = OpConstantTrue %bool
%145 = OpConstantComposite %v2bool %true %true
%162 = OpConstantComposite %v3bool %true %true %true
%177 = OpConstantComposite %v4bool %true %true %true %true
%int_100 = OpConstant %int 100
%194 = OpConstantComposite %v2int %int_0 %int_100
%202 = OpConstantComposite %v3int %int_0 %int_100 %int_0
%210 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
%223 = OpConstantComposite %v2int %int_100 %int_0
%231 = OpConstantComposite %v3int %int_100 %int_0 %int_0
%239 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%411 = OpConstantComposite %v2float %float_0 %float_1
%420 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%429 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%444 = OpConstantComposite %v2float %float_1 %float_0
%453 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%462 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
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
%468 = OpVariable %_ptr_Function_v4float Function
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
%60 = OpLoad %v4int %intGreen
%61 = OpCompositeExtract %int %60 0
%62 = OpLoad %v4int %intRed
%63 = OpCompositeExtract %int %62 0
%64 = OpLoad %v4int %intGreen
%65 = OpCompositeExtract %int %64 0
%66 = OpLoad %v4int %intRed
%67 = OpCompositeExtract %int %66 0
%68 = OpSelect %int %false %67 %65
%69 = OpLoad %v4int %intGreen
%70 = OpCompositeExtract %int %69 0
%71 = OpIEqual %bool %68 %70
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpLoad %v4int %intGreen
%75 = OpVectorShuffle %v2int %74 %74 0 1
%77 = OpLoad %v4int %intRed
%78 = OpVectorShuffle %v2int %77 %77 0 1
%81 = OpLoad %v4int %intGreen
%82 = OpVectorShuffle %v2int %81 %81 0 1
%83 = OpLoad %v4int %intRed
%84 = OpVectorShuffle %v2int %83 %83 0 1
%85 = OpSelect %v2int %80 %84 %82
%86 = OpLoad %v4int %intGreen
%87 = OpVectorShuffle %v2int %86 %86 0 1
%88 = OpIEqual %v2bool %85 %87
%89 = OpAll %bool %88
OpBranch %73
%73 = OpLabel
%90 = OpPhi %bool %false %25 %89 %72
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %v4int %intGreen
%94 = OpVectorShuffle %v3int %93 %93 0 1 2
%96 = OpLoad %v4int %intRed
%97 = OpVectorShuffle %v3int %96 %96 0 1 2
%100 = OpLoad %v4int %intGreen
%101 = OpVectorShuffle %v3int %100 %100 0 1 2
%102 = OpLoad %v4int %intRed
%103 = OpVectorShuffle %v3int %102 %102 0 1 2
%104 = OpSelect %v3int %99 %103 %101
%105 = OpLoad %v4int %intGreen
%106 = OpVectorShuffle %v3int %105 %105 0 1 2
%107 = OpIEqual %v3bool %104 %106
%108 = OpAll %bool %107
OpBranch %92
%92 = OpLabel
%109 = OpPhi %bool %false %73 %108 %91
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpLoad %v4int %intGreen
%113 = OpLoad %v4int %intRed
%116 = OpLoad %v4int %intGreen
%117 = OpLoad %v4int %intRed
%118 = OpSelect %v4int %115 %117 %116
%119 = OpLoad %v4int %intGreen
%120 = OpIEqual %v4bool %118 %119
%121 = OpAll %bool %120
OpBranch %111
%111 = OpLabel
%122 = OpPhi %bool %false %92 %121 %110
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %v4int %intGreen
%126 = OpCompositeExtract %int %125 0
%127 = OpLoad %v4int %intRed
%128 = OpCompositeExtract %int %127 0
%130 = OpLoad %v4int %intGreen
%131 = OpCompositeExtract %int %130 0
%132 = OpLoad %v4int %intRed
%133 = OpCompositeExtract %int %132 0
%134 = OpSelect %int %true %133 %131
%135 = OpLoad %v4int %intRed
%136 = OpCompositeExtract %int %135 0
%137 = OpIEqual %bool %134 %136
OpBranch %124
%124 = OpLabel
%138 = OpPhi %bool %false %111 %137 %123
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%141 = OpLoad %v4int %intGreen
%142 = OpVectorShuffle %v2int %141 %141 0 1
%143 = OpLoad %v4int %intRed
%144 = OpVectorShuffle %v2int %143 %143 0 1
%146 = OpLoad %v4int %intGreen
%147 = OpVectorShuffle %v2int %146 %146 0 1
%148 = OpLoad %v4int %intRed
%149 = OpVectorShuffle %v2int %148 %148 0 1
%150 = OpSelect %v2int %145 %149 %147
%151 = OpLoad %v4int %intRed
%152 = OpVectorShuffle %v2int %151 %151 0 1
%153 = OpIEqual %v2bool %150 %152
%154 = OpAll %bool %153
OpBranch %140
%140 = OpLabel
%155 = OpPhi %bool %false %124 %154 %139
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %v4int %intGreen
%159 = OpVectorShuffle %v3int %158 %158 0 1 2
%160 = OpLoad %v4int %intRed
%161 = OpVectorShuffle %v3int %160 %160 0 1 2
%163 = OpLoad %v4int %intGreen
%164 = OpVectorShuffle %v3int %163 %163 0 1 2
%165 = OpLoad %v4int %intRed
%166 = OpVectorShuffle %v3int %165 %165 0 1 2
%167 = OpSelect %v3int %162 %166 %164
%168 = OpLoad %v4int %intRed
%169 = OpVectorShuffle %v3int %168 %168 0 1 2
%170 = OpIEqual %v3bool %167 %169
%171 = OpAll %bool %170
OpBranch %157
%157 = OpLabel
%172 = OpPhi %bool %false %140 %171 %156
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpLoad %v4int %intGreen
%176 = OpLoad %v4int %intRed
%178 = OpLoad %v4int %intGreen
%179 = OpLoad %v4int %intRed
%180 = OpSelect %v4int %177 %179 %178
%181 = OpLoad %v4int %intRed
%182 = OpIEqual %v4bool %180 %181
%183 = OpAll %bool %182
OpBranch %174
%174 = OpLabel
%184 = OpPhi %bool %false %157 %183 %173
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpLoad %v4int %intGreen
%188 = OpCompositeExtract %int %187 0
%189 = OpIEqual %bool %int_0 %188
OpBranch %186
%186 = OpLabel
%190 = OpPhi %bool %false %174 %189 %185
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%195 = OpLoad %v4int %intGreen
%196 = OpVectorShuffle %v2int %195 %195 0 1
%197 = OpIEqual %v2bool %194 %196
%198 = OpAll %bool %197
OpBranch %192
%192 = OpLabel
%199 = OpPhi %bool %false %186 %198 %191
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%203 = OpLoad %v4int %intGreen
%204 = OpVectorShuffle %v3int %203 %203 0 1 2
%205 = OpIEqual %v3bool %202 %204
%206 = OpAll %bool %205
OpBranch %201
%201 = OpLabel
%207 = OpPhi %bool %false %192 %206 %200
OpSelectionMerge %209 None
OpBranchConditional %207 %208 %209
%208 = OpLabel
%211 = OpLoad %v4int %intGreen
%212 = OpIEqual %v4bool %210 %211
%213 = OpAll %bool %212
OpBranch %209
%209 = OpLabel
%214 = OpPhi %bool %false %201 %213 %208
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%217 = OpLoad %v4int %intRed
%218 = OpCompositeExtract %int %217 0
%219 = OpIEqual %bool %int_100 %218
OpBranch %216
%216 = OpLabel
%220 = OpPhi %bool %false %209 %219 %215
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%224 = OpLoad %v4int %intRed
%225 = OpVectorShuffle %v2int %224 %224 0 1
%226 = OpIEqual %v2bool %223 %225
%227 = OpAll %bool %226
OpBranch %222
%222 = OpLabel
%228 = OpPhi %bool %false %216 %227 %221
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%232 = OpLoad %v4int %intRed
%233 = OpVectorShuffle %v3int %232 %232 0 1 2
%234 = OpIEqual %v3bool %231 %233
%235 = OpAll %bool %234
OpBranch %230
%230 = OpLabel
%236 = OpPhi %bool %false %222 %235 %229
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%240 = OpLoad %v4int %intRed
%241 = OpIEqual %v4bool %239 %240
%242 = OpAll %bool %241
OpBranch %238
%238 = OpLabel
%243 = OpPhi %bool %false %230 %242 %237
OpSelectionMerge %245 None
OpBranchConditional %243 %244 %245
%244 = OpLabel
%246 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%247 = OpLoad %v4float %246
%248 = OpCompositeExtract %float %247 0
%249 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%250 = OpLoad %v4float %249
%251 = OpCompositeExtract %float %250 0
%252 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%253 = OpLoad %v4float %252
%254 = OpCompositeExtract %float %253 0
%255 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%256 = OpLoad %v4float %255
%257 = OpCompositeExtract %float %256 0
%258 = OpSelect %float %false %257 %254
%259 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%260 = OpLoad %v4float %259
%261 = OpCompositeExtract %float %260 0
%262 = OpFOrdEqual %bool %258 %261
OpBranch %245
%245 = OpLabel
%263 = OpPhi %bool %false %238 %262 %244
OpSelectionMerge %265 None
OpBranchConditional %263 %264 %265
%264 = OpLabel
%266 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%267 = OpLoad %v4float %266
%268 = OpVectorShuffle %v2float %267 %267 0 1
%269 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%270 = OpLoad %v4float %269
%271 = OpVectorShuffle %v2float %270 %270 0 1
%272 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%273 = OpLoad %v4float %272
%274 = OpVectorShuffle %v2float %273 %273 0 1
%275 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%276 = OpLoad %v4float %275
%277 = OpVectorShuffle %v2float %276 %276 0 1
%278 = OpSelect %v2float %80 %277 %274
%279 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%280 = OpLoad %v4float %279
%281 = OpVectorShuffle %v2float %280 %280 0 1
%282 = OpFOrdEqual %v2bool %278 %281
%283 = OpAll %bool %282
OpBranch %265
%265 = OpLabel
%284 = OpPhi %bool %false %245 %283 %264
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%287 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%288 = OpLoad %v4float %287
%289 = OpVectorShuffle %v3float %288 %288 0 1 2
%291 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%292 = OpLoad %v4float %291
%293 = OpVectorShuffle %v3float %292 %292 0 1 2
%294 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%295 = OpLoad %v4float %294
%296 = OpVectorShuffle %v3float %295 %295 0 1 2
%297 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%298 = OpLoad %v4float %297
%299 = OpVectorShuffle %v3float %298 %298 0 1 2
%300 = OpSelect %v3float %99 %299 %296
%301 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%302 = OpLoad %v4float %301
%303 = OpVectorShuffle %v3float %302 %302 0 1 2
%304 = OpFOrdEqual %v3bool %300 %303
%305 = OpAll %bool %304
OpBranch %286
%286 = OpLabel
%306 = OpPhi %bool %false %265 %305 %285
OpSelectionMerge %308 None
OpBranchConditional %306 %307 %308
%307 = OpLabel
%309 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%310 = OpLoad %v4float %309
%311 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%312 = OpLoad %v4float %311
%313 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%314 = OpLoad %v4float %313
%315 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%316 = OpLoad %v4float %315
%317 = OpSelect %v4float %115 %316 %314
%318 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%319 = OpLoad %v4float %318
%320 = OpFOrdEqual %v4bool %317 %319
%321 = OpAll %bool %320
OpBranch %308
%308 = OpLabel
%322 = OpPhi %bool %false %286 %321 %307
OpSelectionMerge %324 None
OpBranchConditional %322 %323 %324
%323 = OpLabel
%325 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%326 = OpLoad %v4float %325
%327 = OpCompositeExtract %float %326 0
%328 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%329 = OpLoad %v4float %328
%330 = OpCompositeExtract %float %329 0
%331 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%332 = OpLoad %v4float %331
%333 = OpCompositeExtract %float %332 0
%334 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%335 = OpLoad %v4float %334
%336 = OpCompositeExtract %float %335 0
%337 = OpSelect %float %true %336 %333
%338 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%339 = OpLoad %v4float %338
%340 = OpCompositeExtract %float %339 0
%341 = OpFOrdEqual %bool %337 %340
OpBranch %324
%324 = OpLabel
%342 = OpPhi %bool %false %308 %341 %323
OpSelectionMerge %344 None
OpBranchConditional %342 %343 %344
%343 = OpLabel
%345 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%346 = OpLoad %v4float %345
%347 = OpVectorShuffle %v2float %346 %346 0 1
%348 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%349 = OpLoad %v4float %348
%350 = OpVectorShuffle %v2float %349 %349 0 1
%351 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%352 = OpLoad %v4float %351
%353 = OpVectorShuffle %v2float %352 %352 0 1
%354 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%355 = OpLoad %v4float %354
%356 = OpVectorShuffle %v2float %355 %355 0 1
%357 = OpSelect %v2float %145 %356 %353
%358 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%359 = OpLoad %v4float %358
%360 = OpVectorShuffle %v2float %359 %359 0 1
%361 = OpFOrdEqual %v2bool %357 %360
%362 = OpAll %bool %361
OpBranch %344
%344 = OpLabel
%363 = OpPhi %bool %false %324 %362 %343
OpSelectionMerge %365 None
OpBranchConditional %363 %364 %365
%364 = OpLabel
%366 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%367 = OpLoad %v4float %366
%368 = OpVectorShuffle %v3float %367 %367 0 1 2
%369 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%370 = OpLoad %v4float %369
%371 = OpVectorShuffle %v3float %370 %370 0 1 2
%372 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%373 = OpLoad %v4float %372
%374 = OpVectorShuffle %v3float %373 %373 0 1 2
%375 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%376 = OpLoad %v4float %375
%377 = OpVectorShuffle %v3float %376 %376 0 1 2
%378 = OpSelect %v3float %162 %377 %374
%379 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%380 = OpLoad %v4float %379
%381 = OpVectorShuffle %v3float %380 %380 0 1 2
%382 = OpFOrdEqual %v3bool %378 %381
%383 = OpAll %bool %382
OpBranch %365
%365 = OpLabel
%384 = OpPhi %bool %false %344 %383 %364
OpSelectionMerge %386 None
OpBranchConditional %384 %385 %386
%385 = OpLabel
%387 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%388 = OpLoad %v4float %387
%389 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%390 = OpLoad %v4float %389
%391 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%392 = OpLoad %v4float %391
%393 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%394 = OpLoad %v4float %393
%395 = OpSelect %v4float %177 %394 %392
%396 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%397 = OpLoad %v4float %396
%398 = OpFOrdEqual %v4bool %395 %397
%399 = OpAll %bool %398
OpBranch %386
%386 = OpLabel
%400 = OpPhi %bool %false %365 %399 %385
OpSelectionMerge %402 None
OpBranchConditional %400 %401 %402
%401 = OpLabel
%403 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%404 = OpLoad %v4float %403
%405 = OpCompositeExtract %float %404 0
%406 = OpFOrdEqual %bool %float_0 %405
OpBranch %402
%402 = OpLabel
%407 = OpPhi %bool %false %386 %406 %401
OpSelectionMerge %409 None
OpBranchConditional %407 %408 %409
%408 = OpLabel
%412 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%413 = OpLoad %v4float %412
%414 = OpVectorShuffle %v2float %413 %413 0 1
%415 = OpFOrdEqual %v2bool %411 %414
%416 = OpAll %bool %415
OpBranch %409
%409 = OpLabel
%417 = OpPhi %bool %false %402 %416 %408
OpSelectionMerge %419 None
OpBranchConditional %417 %418 %419
%418 = OpLabel
%421 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%422 = OpLoad %v4float %421
%423 = OpVectorShuffle %v3float %422 %422 0 1 2
%424 = OpFOrdEqual %v3bool %420 %423
%425 = OpAll %bool %424
OpBranch %419
%419 = OpLabel
%426 = OpPhi %bool %false %409 %425 %418
OpSelectionMerge %428 None
OpBranchConditional %426 %427 %428
%427 = OpLabel
%430 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%431 = OpLoad %v4float %430
%432 = OpFOrdEqual %v4bool %429 %431
%433 = OpAll %bool %432
OpBranch %428
%428 = OpLabel
%434 = OpPhi %bool %false %419 %433 %427
OpSelectionMerge %436 None
OpBranchConditional %434 %435 %436
%435 = OpLabel
%437 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%438 = OpLoad %v4float %437
%439 = OpCompositeExtract %float %438 0
%440 = OpFOrdEqual %bool %float_1 %439
OpBranch %436
%436 = OpLabel
%441 = OpPhi %bool %false %428 %440 %435
OpSelectionMerge %443 None
OpBranchConditional %441 %442 %443
%442 = OpLabel
%445 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%446 = OpLoad %v4float %445
%447 = OpVectorShuffle %v2float %446 %446 0 1
%448 = OpFOrdEqual %v2bool %444 %447
%449 = OpAll %bool %448
OpBranch %443
%443 = OpLabel
%450 = OpPhi %bool %false %436 %449 %442
OpSelectionMerge %452 None
OpBranchConditional %450 %451 %452
%451 = OpLabel
%454 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%455 = OpLoad %v4float %454
%456 = OpVectorShuffle %v3float %455 %455 0 1 2
%457 = OpFOrdEqual %v3bool %453 %456
%458 = OpAll %bool %457
OpBranch %452
%452 = OpLabel
%459 = OpPhi %bool %false %443 %458 %451
OpSelectionMerge %461 None
OpBranchConditional %459 %460 %461
%460 = OpLabel
%463 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%464 = OpLoad %v4float %463
%465 = OpFOrdEqual %v4bool %462 %464
%466 = OpAll %bool %465
OpBranch %461
%461 = OpLabel
%467 = OpPhi %bool %false %452 %466 %460
OpSelectionMerge %472 None
OpBranchConditional %467 %470 %471
%470 = OpLabel
%473 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%474 = OpLoad %v4float %473
OpStore %468 %474
OpBranch %472
%471 = OpLabel
%475 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%476 = OpLoad %v4float %475
OpStore %468 %476
OpBranch %472
%472 = OpLabel
%477 = OpLoad %v4float %468
OpReturnValue %477
OpFunctionEnd
