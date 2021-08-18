OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %251 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %283 RelaxedPrecision
OpDecorate %291 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %301 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %305 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %315 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %319 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
OpDecorate %338 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %342 RelaxedPrecision
OpDecorate %349 RelaxedPrecision
OpDecorate %350 RelaxedPrecision
OpDecorate %352 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %355 RelaxedPrecision
OpDecorate %356 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %399 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %407 RelaxedPrecision
OpDecorate %413 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %431 RelaxedPrecision
OpDecorate %433 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %441 RelaxedPrecision
OpDecorate %446 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %457 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %464 RelaxedPrecision
OpDecorate %466 RelaxedPrecision
OpDecorate %476 RelaxedPrecision
OpDecorate %478 RelaxedPrecision
OpDecorate %479 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%83 = OpConstantComposite %v2bool %false %false
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%102 = OpConstantComposite %v3bool %false %false %false
%v4bool = OpTypeVector %bool 4
%118 = OpConstantComposite %v4bool %false %false %false %false
%true = OpConstantTrue %bool
%148 = OpConstantComposite %v2bool %true %true
%165 = OpConstantComposite %v3bool %true %true %true
%180 = OpConstantComposite %v4bool %true %true %true %true
%int_100 = OpConstant %int 100
%196 = OpConstantComposite %v2int %int_0 %int_100
%204 = OpConstantComposite %v3int %int_0 %int_100 %int_0
%212 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
%225 = OpConstantComposite %v2int %int_100 %int_0
%233 = OpConstantComposite %v3int %int_100 %int_0 %int_0
%241 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%413 = OpConstantComposite %v2float %float_0 %float_1
%422 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%431 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%446 = OpConstantComposite %v2float %float_1 %float_0
%455 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%464 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%intGreen = OpVariable %_ptr_Function_v4int Function
%intRed = OpVariable %_ptr_Function_v4int Function
%470 = OpVariable %_ptr_Function_v4float Function
%32 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%35 = OpLoad %v4float %32
%37 = OpVectorTimesScalar %v4float %35 %float_100
%38 = OpCompositeExtract %float %37 0
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %37 1
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %37 2
%43 = OpConvertFToS %int %42
%44 = OpCompositeExtract %float %37 3
%45 = OpConvertFToS %int %44
%46 = OpCompositeConstruct %v4int %39 %41 %43 %45
OpStore %intGreen %46
%48 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%50 = OpLoad %v4float %48
%51 = OpVectorTimesScalar %v4float %50 %float_100
%52 = OpCompositeExtract %float %51 0
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %51 1
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %51 2
%57 = OpConvertFToS %int %56
%58 = OpCompositeExtract %float %51 3
%59 = OpConvertFToS %int %58
%60 = OpCompositeConstruct %v4int %53 %55 %57 %59
OpStore %intRed %60
%63 = OpLoad %v4int %intGreen
%64 = OpCompositeExtract %int %63 0
%65 = OpLoad %v4int %intRed
%66 = OpCompositeExtract %int %65 0
%67 = OpLoad %v4int %intGreen
%68 = OpCompositeExtract %int %67 0
%69 = OpLoad %v4int %intRed
%70 = OpCompositeExtract %int %69 0
%62 = OpSelect %int %false %70 %68
%71 = OpLoad %v4int %intGreen
%72 = OpCompositeExtract %int %71 0
%73 = OpIEqual %bool %62 %72
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpLoad %v4int %intGreen
%78 = OpVectorShuffle %v2int %77 %77 0 1
%80 = OpLoad %v4int %intRed
%81 = OpVectorShuffle %v2int %80 %80 0 1
%84 = OpLoad %v4int %intGreen
%85 = OpVectorShuffle %v2int %84 %84 0 1
%86 = OpLoad %v4int %intRed
%87 = OpVectorShuffle %v2int %86 %86 0 1
%76 = OpSelect %v2int %83 %87 %85
%88 = OpLoad %v4int %intGreen
%89 = OpVectorShuffle %v2int %88 %88 0 1
%90 = OpIEqual %v2bool %76 %89
%91 = OpAll %bool %90
OpBranch %75
%75 = OpLabel
%92 = OpPhi %bool %false %27 %91 %74
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
%95 = OpSelect %v3int %102 %106 %104
%107 = OpLoad %v4int %intGreen
%108 = OpVectorShuffle %v3int %107 %107 0 1 2
%109 = OpIEqual %v3bool %95 %108
%110 = OpAll %bool %109
OpBranch %94
%94 = OpLabel
%111 = OpPhi %bool %false %75 %110 %93
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpLoad %v4int %intGreen
%116 = OpLoad %v4int %intRed
%119 = OpLoad %v4int %intGreen
%120 = OpLoad %v4int %intRed
%114 = OpSelect %v4int %118 %120 %119
%121 = OpLoad %v4int %intGreen
%122 = OpIEqual %v4bool %114 %121
%123 = OpAll %bool %122
OpBranch %113
%113 = OpLabel
%124 = OpPhi %bool %false %94 %123 %112
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpLoad %v4int %intGreen
%129 = OpCompositeExtract %int %128 0
%130 = OpLoad %v4int %intRed
%131 = OpCompositeExtract %int %130 0
%133 = OpLoad %v4int %intGreen
%134 = OpCompositeExtract %int %133 0
%135 = OpLoad %v4int %intRed
%136 = OpCompositeExtract %int %135 0
%127 = OpSelect %int %true %136 %134
%137 = OpLoad %v4int %intRed
%138 = OpCompositeExtract %int %137 0
%139 = OpIEqual %bool %127 %138
OpBranch %126
%126 = OpLabel
%140 = OpPhi %bool %false %113 %139 %125
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpLoad %v4int %intGreen
%145 = OpVectorShuffle %v2int %144 %144 0 1
%146 = OpLoad %v4int %intRed
%147 = OpVectorShuffle %v2int %146 %146 0 1
%149 = OpLoad %v4int %intGreen
%150 = OpVectorShuffle %v2int %149 %149 0 1
%151 = OpLoad %v4int %intRed
%152 = OpVectorShuffle %v2int %151 %151 0 1
%143 = OpSelect %v2int %148 %152 %150
%153 = OpLoad %v4int %intRed
%154 = OpVectorShuffle %v2int %153 %153 0 1
%155 = OpIEqual %v2bool %143 %154
%156 = OpAll %bool %155
OpBranch %142
%142 = OpLabel
%157 = OpPhi %bool %false %126 %156 %141
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%161 = OpLoad %v4int %intGreen
%162 = OpVectorShuffle %v3int %161 %161 0 1 2
%163 = OpLoad %v4int %intRed
%164 = OpVectorShuffle %v3int %163 %163 0 1 2
%166 = OpLoad %v4int %intGreen
%167 = OpVectorShuffle %v3int %166 %166 0 1 2
%168 = OpLoad %v4int %intRed
%169 = OpVectorShuffle %v3int %168 %168 0 1 2
%160 = OpSelect %v3int %165 %169 %167
%170 = OpLoad %v4int %intRed
%171 = OpVectorShuffle %v3int %170 %170 0 1 2
%172 = OpIEqual %v3bool %160 %171
%173 = OpAll %bool %172
OpBranch %159
%159 = OpLabel
%174 = OpPhi %bool %false %142 %173 %158
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpLoad %v4int %intGreen
%179 = OpLoad %v4int %intRed
%181 = OpLoad %v4int %intGreen
%182 = OpLoad %v4int %intRed
%177 = OpSelect %v4int %180 %182 %181
%183 = OpLoad %v4int %intRed
%184 = OpIEqual %v4bool %177 %183
%185 = OpAll %bool %184
OpBranch %176
%176 = OpLabel
%186 = OpPhi %bool %false %159 %185 %175
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpLoad %v4int %intGreen
%190 = OpCompositeExtract %int %189 0
%191 = OpIEqual %bool %int_0 %190
OpBranch %188
%188 = OpLabel
%192 = OpPhi %bool %false %176 %191 %187
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%197 = OpLoad %v4int %intGreen
%198 = OpVectorShuffle %v2int %197 %197 0 1
%199 = OpIEqual %v2bool %196 %198
%200 = OpAll %bool %199
OpBranch %194
%194 = OpLabel
%201 = OpPhi %bool %false %188 %200 %193
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%205 = OpLoad %v4int %intGreen
%206 = OpVectorShuffle %v3int %205 %205 0 1 2
%207 = OpIEqual %v3bool %204 %206
%208 = OpAll %bool %207
OpBranch %203
%203 = OpLabel
%209 = OpPhi %bool %false %194 %208 %202
OpSelectionMerge %211 None
OpBranchConditional %209 %210 %211
%210 = OpLabel
%213 = OpLoad %v4int %intGreen
%214 = OpIEqual %v4bool %212 %213
%215 = OpAll %bool %214
OpBranch %211
%211 = OpLabel
%216 = OpPhi %bool %false %203 %215 %210
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpLoad %v4int %intRed
%220 = OpCompositeExtract %int %219 0
%221 = OpIEqual %bool %int_100 %220
OpBranch %218
%218 = OpLabel
%222 = OpPhi %bool %false %211 %221 %217
OpSelectionMerge %224 None
OpBranchConditional %222 %223 %224
%223 = OpLabel
%226 = OpLoad %v4int %intRed
%227 = OpVectorShuffle %v2int %226 %226 0 1
%228 = OpIEqual %v2bool %225 %227
%229 = OpAll %bool %228
OpBranch %224
%224 = OpLabel
%230 = OpPhi %bool %false %218 %229 %223
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%234 = OpLoad %v4int %intRed
%235 = OpVectorShuffle %v3int %234 %234 0 1 2
%236 = OpIEqual %v3bool %233 %235
%237 = OpAll %bool %236
OpBranch %232
%232 = OpLabel
%238 = OpPhi %bool %false %224 %237 %231
OpSelectionMerge %240 None
OpBranchConditional %238 %239 %240
%239 = OpLabel
%242 = OpLoad %v4int %intRed
%243 = OpIEqual %v4bool %241 %242
%244 = OpAll %bool %243
OpBranch %240
%240 = OpLabel
%245 = OpPhi %bool %false %232 %244 %239
OpSelectionMerge %247 None
OpBranchConditional %245 %246 %247
%246 = OpLabel
%249 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%250 = OpLoad %v4float %249
%251 = OpCompositeExtract %float %250 0
%252 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%253 = OpLoad %v4float %252
%254 = OpCompositeExtract %float %253 0
%255 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%256 = OpLoad %v4float %255
%257 = OpCompositeExtract %float %256 0
%258 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%259 = OpLoad %v4float %258
%260 = OpCompositeExtract %float %259 0
%248 = OpSelect %float %false %260 %257
%261 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%262 = OpLoad %v4float %261
%263 = OpCompositeExtract %float %262 0
%264 = OpFOrdEqual %bool %248 %263
OpBranch %247
%247 = OpLabel
%265 = OpPhi %bool %false %240 %264 %246
OpSelectionMerge %267 None
OpBranchConditional %265 %266 %267
%266 = OpLabel
%269 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%270 = OpLoad %v4float %269
%271 = OpVectorShuffle %v2float %270 %270 0 1
%272 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%273 = OpLoad %v4float %272
%274 = OpVectorShuffle %v2float %273 %273 0 1
%275 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%276 = OpLoad %v4float %275
%277 = OpVectorShuffle %v2float %276 %276 0 1
%278 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%279 = OpLoad %v4float %278
%280 = OpVectorShuffle %v2float %279 %279 0 1
%268 = OpSelect %v2float %83 %280 %277
%281 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%282 = OpLoad %v4float %281
%283 = OpVectorShuffle %v2float %282 %282 0 1
%284 = OpFOrdEqual %v2bool %268 %283
%285 = OpAll %bool %284
OpBranch %267
%267 = OpLabel
%286 = OpPhi %bool %false %247 %285 %266
OpSelectionMerge %288 None
OpBranchConditional %286 %287 %288
%287 = OpLabel
%290 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%291 = OpLoad %v4float %290
%292 = OpVectorShuffle %v3float %291 %291 0 1 2
%294 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%295 = OpLoad %v4float %294
%296 = OpVectorShuffle %v3float %295 %295 0 1 2
%297 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%298 = OpLoad %v4float %297
%299 = OpVectorShuffle %v3float %298 %298 0 1 2
%300 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%301 = OpLoad %v4float %300
%302 = OpVectorShuffle %v3float %301 %301 0 1 2
%289 = OpSelect %v3float %102 %302 %299
%303 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%304 = OpLoad %v4float %303
%305 = OpVectorShuffle %v3float %304 %304 0 1 2
%306 = OpFOrdEqual %v3bool %289 %305
%307 = OpAll %bool %306
OpBranch %288
%288 = OpLabel
%308 = OpPhi %bool %false %267 %307 %287
OpSelectionMerge %310 None
OpBranchConditional %308 %309 %310
%309 = OpLabel
%312 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%313 = OpLoad %v4float %312
%314 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%315 = OpLoad %v4float %314
%316 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%317 = OpLoad %v4float %316
%318 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%319 = OpLoad %v4float %318
%311 = OpSelect %v4float %118 %319 %317
%320 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%321 = OpLoad %v4float %320
%322 = OpFOrdEqual %v4bool %311 %321
%323 = OpAll %bool %322
OpBranch %310
%310 = OpLabel
%324 = OpPhi %bool %false %288 %323 %309
OpSelectionMerge %326 None
OpBranchConditional %324 %325 %326
%325 = OpLabel
%328 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%329 = OpLoad %v4float %328
%330 = OpCompositeExtract %float %329 0
%331 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%332 = OpLoad %v4float %331
%333 = OpCompositeExtract %float %332 0
%334 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%335 = OpLoad %v4float %334
%336 = OpCompositeExtract %float %335 0
%337 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%338 = OpLoad %v4float %337
%339 = OpCompositeExtract %float %338 0
%327 = OpSelect %float %true %339 %336
%340 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%341 = OpLoad %v4float %340
%342 = OpCompositeExtract %float %341 0
%343 = OpFOrdEqual %bool %327 %342
OpBranch %326
%326 = OpLabel
%344 = OpPhi %bool %false %310 %343 %325
OpSelectionMerge %346 None
OpBranchConditional %344 %345 %346
%345 = OpLabel
%348 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%349 = OpLoad %v4float %348
%350 = OpVectorShuffle %v2float %349 %349 0 1
%351 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%352 = OpLoad %v4float %351
%353 = OpVectorShuffle %v2float %352 %352 0 1
%354 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%355 = OpLoad %v4float %354
%356 = OpVectorShuffle %v2float %355 %355 0 1
%357 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%358 = OpLoad %v4float %357
%359 = OpVectorShuffle %v2float %358 %358 0 1
%347 = OpSelect %v2float %148 %359 %356
%360 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%361 = OpLoad %v4float %360
%362 = OpVectorShuffle %v2float %361 %361 0 1
%363 = OpFOrdEqual %v2bool %347 %362
%364 = OpAll %bool %363
OpBranch %346
%346 = OpLabel
%365 = OpPhi %bool %false %326 %364 %345
OpSelectionMerge %367 None
OpBranchConditional %365 %366 %367
%366 = OpLabel
%369 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%370 = OpLoad %v4float %369
%371 = OpVectorShuffle %v3float %370 %370 0 1 2
%372 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%373 = OpLoad %v4float %372
%374 = OpVectorShuffle %v3float %373 %373 0 1 2
%375 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%376 = OpLoad %v4float %375
%377 = OpVectorShuffle %v3float %376 %376 0 1 2
%378 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%379 = OpLoad %v4float %378
%380 = OpVectorShuffle %v3float %379 %379 0 1 2
%368 = OpSelect %v3float %165 %380 %377
%381 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%382 = OpLoad %v4float %381
%383 = OpVectorShuffle %v3float %382 %382 0 1 2
%384 = OpFOrdEqual %v3bool %368 %383
%385 = OpAll %bool %384
OpBranch %367
%367 = OpLabel
%386 = OpPhi %bool %false %346 %385 %366
OpSelectionMerge %388 None
OpBranchConditional %386 %387 %388
%387 = OpLabel
%390 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%391 = OpLoad %v4float %390
%392 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%393 = OpLoad %v4float %392
%394 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%395 = OpLoad %v4float %394
%396 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%397 = OpLoad %v4float %396
%389 = OpSelect %v4float %180 %397 %395
%398 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%399 = OpLoad %v4float %398
%400 = OpFOrdEqual %v4bool %389 %399
%401 = OpAll %bool %400
OpBranch %388
%388 = OpLabel
%402 = OpPhi %bool %false %367 %401 %387
OpSelectionMerge %404 None
OpBranchConditional %402 %403 %404
%403 = OpLabel
%405 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%406 = OpLoad %v4float %405
%407 = OpCompositeExtract %float %406 0
%408 = OpFOrdEqual %bool %float_0 %407
OpBranch %404
%404 = OpLabel
%409 = OpPhi %bool %false %388 %408 %403
OpSelectionMerge %411 None
OpBranchConditional %409 %410 %411
%410 = OpLabel
%414 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%415 = OpLoad %v4float %414
%416 = OpVectorShuffle %v2float %415 %415 0 1
%417 = OpFOrdEqual %v2bool %413 %416
%418 = OpAll %bool %417
OpBranch %411
%411 = OpLabel
%419 = OpPhi %bool %false %404 %418 %410
OpSelectionMerge %421 None
OpBranchConditional %419 %420 %421
%420 = OpLabel
%423 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%424 = OpLoad %v4float %423
%425 = OpVectorShuffle %v3float %424 %424 0 1 2
%426 = OpFOrdEqual %v3bool %422 %425
%427 = OpAll %bool %426
OpBranch %421
%421 = OpLabel
%428 = OpPhi %bool %false %411 %427 %420
OpSelectionMerge %430 None
OpBranchConditional %428 %429 %430
%429 = OpLabel
%432 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%433 = OpLoad %v4float %432
%434 = OpFOrdEqual %v4bool %431 %433
%435 = OpAll %bool %434
OpBranch %430
%430 = OpLabel
%436 = OpPhi %bool %false %421 %435 %429
OpSelectionMerge %438 None
OpBranchConditional %436 %437 %438
%437 = OpLabel
%439 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%440 = OpLoad %v4float %439
%441 = OpCompositeExtract %float %440 0
%442 = OpFOrdEqual %bool %float_1 %441
OpBranch %438
%438 = OpLabel
%443 = OpPhi %bool %false %430 %442 %437
OpSelectionMerge %445 None
OpBranchConditional %443 %444 %445
%444 = OpLabel
%447 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%448 = OpLoad %v4float %447
%449 = OpVectorShuffle %v2float %448 %448 0 1
%450 = OpFOrdEqual %v2bool %446 %449
%451 = OpAll %bool %450
OpBranch %445
%445 = OpLabel
%452 = OpPhi %bool %false %438 %451 %444
OpSelectionMerge %454 None
OpBranchConditional %452 %453 %454
%453 = OpLabel
%456 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%457 = OpLoad %v4float %456
%458 = OpVectorShuffle %v3float %457 %457 0 1 2
%459 = OpFOrdEqual %v3bool %455 %458
%460 = OpAll %bool %459
OpBranch %454
%454 = OpLabel
%461 = OpPhi %bool %false %445 %460 %453
OpSelectionMerge %463 None
OpBranchConditional %461 %462 %463
%462 = OpLabel
%465 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%466 = OpLoad %v4float %465
%467 = OpFOrdEqual %v4bool %464 %466
%468 = OpAll %bool %467
OpBranch %463
%463 = OpLabel
%469 = OpPhi %bool %false %454 %468 %462
OpSelectionMerge %474 None
OpBranchConditional %469 %472 %473
%472 = OpLabel
%475 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%476 = OpLoad %v4float %475
OpStore %470 %476
OpBranch %474
%473 = OpLabel
%477 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%478 = OpLoad %v4float %477
OpStore %470 %478
OpBranch %474
%474 = OpLabel
%479 = OpLoad %v4float %470
OpReturnValue %479
OpFunctionEnd
