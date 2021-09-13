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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %304 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %422 RelaxedPrecision
OpDecorate %425 RelaxedPrecision
OpDecorate %426 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%float_4 = OpConstant %float 4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%mat2v3float = OpTypeMatrix %v3float 2
%v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat3v4float = OpTypeMatrix %v4float 3
%mat4v2float = OpTypeMatrix %v2float 4
%mat4v3float = OpTypeMatrix %v3float 4
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%415 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpLoad %v4float %f4
%42 = OpVectorShuffle %v3float %41 %41 0 1 2
%45 = OpCompositeExtract %float %42 0
%46 = OpCompositeExtract %float %42 1
%47 = OpCompositeConstruct %v2float %45 %46
%48 = OpCompositeExtract %float %42 2
%49 = OpCompositeConstruct %v2float %48 %float_4
%50 = OpCompositeConstruct %mat2v2float %47 %49
%54 = OpCompositeConstruct %v2float %float_1 %float_2
%55 = OpCompositeConstruct %v2float %float_3 %float_4
%56 = OpCompositeConstruct %mat2v2float %54 %55
%58 = OpCompositeExtract %v2float %50 0
%59 = OpCompositeExtract %v2float %56 0
%60 = OpFOrdEqual %v2bool %58 %59
%61 = OpAll %bool %60
%62 = OpCompositeExtract %v2float %50 1
%63 = OpCompositeExtract %v2float %56 1
%64 = OpFOrdEqual %v2bool %62 %63
%65 = OpAll %bool %64
%66 = OpLogicalAnd %bool %61 %65
OpStore %ok %66
%68 = OpLoad %bool %ok
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %v4float %f4
%72 = OpLoad %v4float %f4
%73 = OpVectorShuffle %v2float %72 %72 0 1
%74 = OpCompositeExtract %float %71 0
%75 = OpCompositeExtract %float %71 1
%76 = OpCompositeExtract %float %71 2
%77 = OpCompositeConstruct %v3float %74 %75 %76
%78 = OpCompositeExtract %float %71 3
%79 = OpCompositeExtract %float %73 0
%80 = OpCompositeExtract %float %73 1
%81 = OpCompositeConstruct %v3float %78 %79 %80
%82 = OpCompositeConstruct %mat2v3float %77 %81
%84 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%85 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%86 = OpCompositeConstruct %mat2v3float %84 %85
%88 = OpCompositeExtract %v3float %82 0
%89 = OpCompositeExtract %v3float %86 0
%90 = OpFOrdEqual %v3bool %88 %89
%91 = OpAll %bool %90
%92 = OpCompositeExtract %v3float %82 1
%93 = OpCompositeExtract %v3float %86 1
%94 = OpFOrdEqual %v3bool %92 %93
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %91 %95
OpBranch %70
%70 = OpLabel
%97 = OpPhi %bool %false %26 %96 %69
OpStore %ok %97
%98 = OpLoad %bool %ok
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %v4float %f4
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpLoad %v4float %f4
%104 = OpVectorShuffle %v4float %103 %103 3 0 1 2
%105 = OpLoad %v4float %f4
%106 = OpCompositeExtract %float %105 3
%107 = OpCompositeExtract %float %102 0
%108 = OpCompositeExtract %float %102 1
%109 = OpCompositeExtract %float %102 2
%110 = OpCompositeExtract %float %104 0
%111 = OpCompositeConstruct %v4float %107 %108 %109 %110
%112 = OpCompositeExtract %float %104 1
%113 = OpCompositeExtract %float %104 2
%114 = OpCompositeExtract %float %104 3
%115 = OpCompositeConstruct %v4float %112 %113 %114 %106
%116 = OpCompositeConstruct %mat2v4float %111 %115
%118 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%119 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%120 = OpCompositeConstruct %mat2v4float %118 %119
%122 = OpCompositeExtract %v4float %116 0
%123 = OpCompositeExtract %v4float %120 0
%124 = OpFOrdEqual %v4bool %122 %123
%125 = OpAll %bool %124
%126 = OpCompositeExtract %v4float %116 1
%127 = OpCompositeExtract %v4float %120 1
%128 = OpFOrdEqual %v4bool %126 %127
%129 = OpAll %bool %128
%130 = OpLogicalAnd %bool %125 %129
OpBranch %100
%100 = OpLabel
%131 = OpPhi %bool %false %70 %130 %99
OpStore %ok %131
%132 = OpLoad %bool %ok
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpLoad %v4float %f4
%136 = OpCompositeExtract %float %135 0
%137 = OpLoad %v4float %f4
%138 = OpVectorShuffle %v4float %137 %137 1 2 3 0
%139 = OpLoad %v4float %f4
%140 = OpCompositeExtract %float %139 1
%141 = OpCompositeExtract %float %138 0
%142 = OpCompositeConstruct %v2float %136 %141
%143 = OpCompositeExtract %float %138 1
%144 = OpCompositeExtract %float %138 2
%145 = OpCompositeConstruct %v2float %143 %144
%146 = OpCompositeExtract %float %138 3
%147 = OpCompositeConstruct %v2float %146 %140
%148 = OpCompositeConstruct %mat3v2float %142 %145 %147
%150 = OpCompositeConstruct %v2float %float_1 %float_2
%151 = OpCompositeConstruct %v2float %float_3 %float_4
%152 = OpCompositeConstruct %v2float %float_1 %float_2
%153 = OpCompositeConstruct %mat3v2float %150 %151 %152
%154 = OpCompositeExtract %v2float %148 0
%155 = OpCompositeExtract %v2float %153 0
%156 = OpFOrdEqual %v2bool %154 %155
%157 = OpAll %bool %156
%158 = OpCompositeExtract %v2float %148 1
%159 = OpCompositeExtract %v2float %153 1
%160 = OpFOrdEqual %v2bool %158 %159
%161 = OpAll %bool %160
%162 = OpLogicalAnd %bool %157 %161
%163 = OpCompositeExtract %v2float %148 2
%164 = OpCompositeExtract %v2float %153 2
%165 = OpFOrdEqual %v2bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %162 %166
OpBranch %134
%134 = OpLabel
%168 = OpPhi %bool %false %100 %167 %133
OpStore %ok %168
%169 = OpLoad %bool %ok
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpLoad %v4float %f4
%173 = OpVectorShuffle %v2float %172 %172 0 1
%174 = OpLoad %v4float %f4
%175 = OpVectorShuffle %v2float %174 %174 2 3
%176 = OpLoad %v4float %f4
%177 = OpLoad %v4float %f4
%178 = OpCompositeExtract %float %177 0
%179 = OpCompositeExtract %float %173 0
%180 = OpCompositeExtract %float %173 1
%181 = OpCompositeExtract %float %175 0
%182 = OpCompositeConstruct %v3float %179 %180 %181
%183 = OpCompositeExtract %float %175 1
%184 = OpCompositeExtract %float %176 0
%185 = OpCompositeExtract %float %176 1
%186 = OpCompositeConstruct %v3float %183 %184 %185
%187 = OpCompositeExtract %float %176 2
%188 = OpCompositeExtract %float %176 3
%189 = OpCompositeConstruct %v3float %187 %188 %178
%190 = OpCompositeConstruct %mat3v3float %182 %186 %189
%192 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%193 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%194 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%195 = OpCompositeConstruct %mat3v3float %192 %193 %194
%196 = OpCompositeExtract %v3float %190 0
%197 = OpCompositeExtract %v3float %195 0
%198 = OpFOrdEqual %v3bool %196 %197
%199 = OpAll %bool %198
%200 = OpCompositeExtract %v3float %190 1
%201 = OpCompositeExtract %v3float %195 1
%202 = OpFOrdEqual %v3bool %200 %201
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %199 %203
%205 = OpCompositeExtract %v3float %190 2
%206 = OpCompositeExtract %v3float %195 2
%207 = OpFOrdEqual %v3bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
OpBranch %171
%171 = OpLabel
%210 = OpPhi %bool %false %134 %209 %170
OpStore %ok %210
%211 = OpLoad %bool %ok
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%214 = OpLoad %v4float %f4
%215 = OpCompositeExtract %float %214 0
%216 = OpLoad %v4float %f4
%217 = OpVectorShuffle %v4float %216 %216 1 2 3 0
%218 = OpLoad %v4float %f4
%219 = OpVectorShuffle %v4float %218 %218 1 2 3 0
%220 = OpLoad %v4float %f4
%221 = OpVectorShuffle %v3float %220 %220 1 2 3
%222 = OpCompositeExtract %float %217 0
%223 = OpCompositeExtract %float %217 1
%224 = OpCompositeExtract %float %217 2
%225 = OpCompositeConstruct %v4float %215 %222 %223 %224
%226 = OpCompositeExtract %float %217 3
%227 = OpCompositeExtract %float %219 0
%228 = OpCompositeExtract %float %219 1
%229 = OpCompositeExtract %float %219 2
%230 = OpCompositeConstruct %v4float %226 %227 %228 %229
%231 = OpCompositeExtract %float %219 3
%232 = OpCompositeExtract %float %221 0
%233 = OpCompositeExtract %float %221 1
%234 = OpCompositeExtract %float %221 2
%235 = OpCompositeConstruct %v4float %231 %232 %233 %234
%236 = OpCompositeConstruct %mat3v4float %225 %230 %235
%238 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%239 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%240 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%241 = OpCompositeConstruct %mat3v4float %238 %239 %240
%242 = OpCompositeExtract %v4float %236 0
%243 = OpCompositeExtract %v4float %241 0
%244 = OpFOrdEqual %v4bool %242 %243
%245 = OpAll %bool %244
%246 = OpCompositeExtract %v4float %236 1
%247 = OpCompositeExtract %v4float %241 1
%248 = OpFOrdEqual %v4bool %246 %247
%249 = OpAll %bool %248
%250 = OpLogicalAnd %bool %245 %249
%251 = OpCompositeExtract %v4float %236 2
%252 = OpCompositeExtract %v4float %241 2
%253 = OpFOrdEqual %v4bool %251 %252
%254 = OpAll %bool %253
%255 = OpLogicalAnd %bool %250 %254
OpBranch %213
%213 = OpLabel
%256 = OpPhi %bool %false %171 %255 %212
OpStore %ok %256
%257 = OpLoad %bool %ok
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%260 = OpLoad %v4float %f4
%261 = OpVectorShuffle %v3float %260 %260 0 1 2
%262 = OpLoad %v4float %f4
%263 = OpVectorShuffle %v4float %262 %262 3 0 1 2
%264 = OpLoad %v4float %f4
%265 = OpCompositeExtract %float %264 3
%266 = OpCompositeExtract %float %261 0
%267 = OpCompositeExtract %float %261 1
%268 = OpCompositeConstruct %v2float %266 %267
%269 = OpCompositeExtract %float %261 2
%270 = OpCompositeExtract %float %263 0
%271 = OpCompositeConstruct %v2float %269 %270
%272 = OpCompositeExtract %float %263 1
%273 = OpCompositeExtract %float %263 2
%274 = OpCompositeConstruct %v2float %272 %273
%275 = OpCompositeExtract %float %263 3
%276 = OpCompositeConstruct %v2float %275 %265
%277 = OpCompositeConstruct %mat4v2float %268 %271 %274 %276
%279 = OpCompositeConstruct %v2float %float_1 %float_2
%280 = OpCompositeConstruct %v2float %float_3 %float_4
%281 = OpCompositeConstruct %v2float %float_1 %float_2
%282 = OpCompositeConstruct %v2float %float_3 %float_4
%283 = OpCompositeConstruct %mat4v2float %279 %280 %281 %282
%284 = OpCompositeExtract %v2float %277 0
%285 = OpCompositeExtract %v2float %283 0
%286 = OpFOrdEqual %v2bool %284 %285
%287 = OpAll %bool %286
%288 = OpCompositeExtract %v2float %277 1
%289 = OpCompositeExtract %v2float %283 1
%290 = OpFOrdEqual %v2bool %288 %289
%291 = OpAll %bool %290
%292 = OpLogicalAnd %bool %287 %291
%293 = OpCompositeExtract %v2float %277 2
%294 = OpCompositeExtract %v2float %283 2
%295 = OpFOrdEqual %v2bool %293 %294
%296 = OpAll %bool %295
%297 = OpLogicalAnd %bool %292 %296
%298 = OpCompositeExtract %v2float %277 3
%299 = OpCompositeExtract %v2float %283 3
%300 = OpFOrdEqual %v2bool %298 %299
%301 = OpAll %bool %300
%302 = OpLogicalAnd %bool %297 %301
OpBranch %259
%259 = OpLabel
%303 = OpPhi %bool %false %213 %302 %258
OpStore %ok %303
%304 = OpLoad %bool %ok
OpSelectionMerge %306 None
OpBranchConditional %304 %305 %306
%305 = OpLabel
%307 = OpLoad %v4float %f4
%308 = OpCompositeExtract %float %307 0
%309 = OpLoad %v4float %f4
%310 = OpVectorShuffle %v4float %309 %309 1 2 3 0
%311 = OpLoad %v4float %f4
%312 = OpVectorShuffle %v4float %311 %311 1 2 3 0
%313 = OpLoad %v4float %f4
%314 = OpVectorShuffle %v3float %313 %313 1 2 3
%315 = OpCompositeExtract %float %310 0
%316 = OpCompositeExtract %float %310 1
%317 = OpCompositeConstruct %v3float %308 %315 %316
%318 = OpCompositeExtract %float %310 2
%319 = OpCompositeExtract %float %310 3
%320 = OpCompositeExtract %float %312 0
%321 = OpCompositeConstruct %v3float %318 %319 %320
%322 = OpCompositeExtract %float %312 1
%323 = OpCompositeExtract %float %312 2
%324 = OpCompositeExtract %float %312 3
%325 = OpCompositeConstruct %v3float %322 %323 %324
%326 = OpCompositeConstruct %mat4v3float %317 %321 %325 %314
%328 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%329 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%330 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%331 = OpCompositeConstruct %v3float %float_2 %float_3 %float_4
%332 = OpCompositeConstruct %mat4v3float %328 %329 %330 %331
%333 = OpCompositeExtract %v3float %326 0
%334 = OpCompositeExtract %v3float %332 0
%335 = OpFOrdEqual %v3bool %333 %334
%336 = OpAll %bool %335
%337 = OpCompositeExtract %v3float %326 1
%338 = OpCompositeExtract %v3float %332 1
%339 = OpFOrdEqual %v3bool %337 %338
%340 = OpAll %bool %339
%341 = OpLogicalAnd %bool %336 %340
%342 = OpCompositeExtract %v3float %326 2
%343 = OpCompositeExtract %v3float %332 2
%344 = OpFOrdEqual %v3bool %342 %343
%345 = OpAll %bool %344
%346 = OpLogicalAnd %bool %341 %345
%347 = OpCompositeExtract %v3float %326 3
%348 = OpCompositeExtract %v3float %332 3
%349 = OpFOrdEqual %v3bool %347 %348
%350 = OpAll %bool %349
%351 = OpLogicalAnd %bool %346 %350
OpBranch %306
%306 = OpLabel
%352 = OpPhi %bool %false %259 %351 %305
OpStore %ok %352
%353 = OpLoad %bool %ok
OpSelectionMerge %355 None
OpBranchConditional %353 %354 %355
%354 = OpLabel
%356 = OpLoad %v4float %f4
%357 = OpVectorShuffle %v2float %356 %356 0 1
%358 = OpLoad %v4float %f4
%359 = OpVectorShuffle %v4float %358 %358 2 3 0 1
%360 = OpLoad %v4float %f4
%361 = OpVectorShuffle %v3float %360 %360 2 3 0
%362 = OpLoad %v4float %f4
%363 = OpVectorShuffle %v2float %362 %362 1 2
%364 = OpLoad %v4float %f4
%365 = OpVectorShuffle %v4float %364 %364 3 0 1 2
%366 = OpLoad %v4float %f4
%367 = OpCompositeExtract %float %366 3
%368 = OpCompositeExtract %float %357 0
%369 = OpCompositeExtract %float %357 1
%370 = OpCompositeExtract %float %359 0
%371 = OpCompositeExtract %float %359 1
%372 = OpCompositeConstruct %v4float %368 %369 %370 %371
%373 = OpCompositeExtract %float %359 2
%374 = OpCompositeExtract %float %359 3
%375 = OpCompositeExtract %float %361 0
%376 = OpCompositeExtract %float %361 1
%377 = OpCompositeConstruct %v4float %373 %374 %375 %376
%378 = OpCompositeExtract %float %361 2
%379 = OpCompositeExtract %float %363 0
%380 = OpCompositeExtract %float %363 1
%381 = OpCompositeExtract %float %365 0
%382 = OpCompositeConstruct %v4float %378 %379 %380 %381
%383 = OpCompositeExtract %float %365 1
%384 = OpCompositeExtract %float %365 2
%385 = OpCompositeExtract %float %365 3
%386 = OpCompositeConstruct %v4float %383 %384 %385 %367
%387 = OpCompositeConstruct %mat4v4float %372 %377 %382 %386
%389 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%390 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%391 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%392 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%393 = OpCompositeConstruct %mat4v4float %389 %390 %391 %392
%394 = OpCompositeExtract %v4float %387 0
%395 = OpCompositeExtract %v4float %393 0
%396 = OpFOrdEqual %v4bool %394 %395
%397 = OpAll %bool %396
%398 = OpCompositeExtract %v4float %387 1
%399 = OpCompositeExtract %v4float %393 1
%400 = OpFOrdEqual %v4bool %398 %399
%401 = OpAll %bool %400
%402 = OpLogicalAnd %bool %397 %401
%403 = OpCompositeExtract %v4float %387 2
%404 = OpCompositeExtract %v4float %393 2
%405 = OpFOrdEqual %v4bool %403 %404
%406 = OpAll %bool %405
%407 = OpLogicalAnd %bool %402 %406
%408 = OpCompositeExtract %v4float %387 3
%409 = OpCompositeExtract %v4float %393 3
%410 = OpFOrdEqual %v4bool %408 %409
%411 = OpAll %bool %410
%412 = OpLogicalAnd %bool %407 %411
OpBranch %355
%355 = OpLabel
%413 = OpPhi %bool %false %306 %412 %354
OpStore %ok %413
%414 = OpLoad %bool %ok
OpSelectionMerge %418 None
OpBranchConditional %414 %416 %417
%416 = OpLabel
%419 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%422 = OpLoad %v4float %419
OpStore %415 %422
OpBranch %418
%417 = OpLabel
%423 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%425 = OpLoad %v4float %423
OpStore %415 %425
OpBranch %418
%418 = OpLabel
%426 = OpLoad %v4float %415
OpReturnValue %426
OpFunctionEnd
