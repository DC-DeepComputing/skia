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
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
OpDecorate %252 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %272 RelaxedPrecision
OpDecorate %273 RelaxedPrecision
OpDecorate %275 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %277 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %287 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %290 RelaxedPrecision
OpDecorate %292 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %299 RelaxedPrecision
OpDecorate %300 RelaxedPrecision
OpDecorate %307 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
OpDecorate %313 RelaxedPrecision
OpDecorate %314 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %317 RelaxedPrecision
OpDecorate %318 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %328 RelaxedPrecision
OpDecorate %330 RelaxedPrecision
OpDecorate %332 RelaxedPrecision
OpDecorate %334 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %345 RelaxedPrecision
OpDecorate %353 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %404 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %416 RelaxedPrecision
OpDecorate %417 RelaxedPrecision
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
%68 = OpConstantComposite %v2bool %false %false
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%82 = OpConstantComposite %v3bool %false %false %false
%v4bool = OpTypeVector %bool 4
%93 = OpConstantComposite %v4bool %false %false %false %false
%true = OpConstantTrue %bool
%108 = OpConstantComposite %v2bool %true %true
%120 = OpConstantComposite %v3bool %true %true %true
%130 = OpConstantComposite %v4bool %true %true %true %true
%int_100 = OpConstant %int 100
%142 = OpConstantComposite %v2int %int_0 %int_100
%149 = OpConstantComposite %v3int %int_0 %int_100 %int_0
%156 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
%166 = OpConstantComposite %v2int %int_100 %int_0
%173 = OpConstantComposite %v3int %int_100 %int_0 %int_0
%180 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
%v3float = OpTypeVector %float 3
%float_1 = OpConstant %float 1
%351 = OpConstantComposite %v2float %float_0 %float_1
%360 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%369 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%384 = OpConstantComposite %v2float %float_1 %float_0
%393 = OpConstantComposite %v3float %float_1 %float_0 %float_0
%402 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
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
%408 = OpVariable %_ptr_Function_v4float Function
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
%60 = OpSelect %int %false %51 %37
%61 = OpIEqual %bool %60 %37
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpVectorShuffle %v2int %44 %44 0 1
%66 = OpVectorShuffle %v2int %58 %58 0 1
%69 = OpVectorShuffle %v2int %44 %44 0 1
%70 = OpVectorShuffle %v2int %58 %58 0 1
%71 = OpSelect %v2int %68 %70 %69
%72 = OpVectorShuffle %v2int %44 %44 0 1
%73 = OpIEqual %v2bool %71 %72
%74 = OpAll %bool %73
OpBranch %63
%63 = OpLabel
%75 = OpPhi %bool %false %25 %74 %62
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpVectorShuffle %v3int %44 %44 0 1 2
%80 = OpVectorShuffle %v3int %58 %58 0 1 2
%83 = OpVectorShuffle %v3int %44 %44 0 1 2
%84 = OpVectorShuffle %v3int %58 %58 0 1 2
%85 = OpSelect %v3int %82 %84 %83
%86 = OpVectorShuffle %v3int %44 %44 0 1 2
%87 = OpIEqual %v3bool %85 %86
%88 = OpAll %bool %87
OpBranch %77
%77 = OpLabel
%89 = OpPhi %bool %false %63 %88 %76
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%94 = OpSelect %v4int %93 %58 %44
%95 = OpIEqual %v4bool %94 %44
%96 = OpAll %bool %95
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %false %77 %96 %90
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpSelect %int %true %51 %37
%102 = OpIEqual %bool %101 %51
OpBranch %99
%99 = OpLabel
%103 = OpPhi %bool %false %91 %102 %98
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpVectorShuffle %v2int %44 %44 0 1
%107 = OpVectorShuffle %v2int %58 %58 0 1
%109 = OpVectorShuffle %v2int %44 %44 0 1
%110 = OpVectorShuffle %v2int %58 %58 0 1
%111 = OpSelect %v2int %108 %110 %109
%112 = OpVectorShuffle %v2int %58 %58 0 1
%113 = OpIEqual %v2bool %111 %112
%114 = OpAll %bool %113
OpBranch %105
%105 = OpLabel
%115 = OpPhi %bool %false %99 %114 %104
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpVectorShuffle %v3int %44 %44 0 1 2
%119 = OpVectorShuffle %v3int %58 %58 0 1 2
%121 = OpVectorShuffle %v3int %44 %44 0 1 2
%122 = OpVectorShuffle %v3int %58 %58 0 1 2
%123 = OpSelect %v3int %120 %122 %121
%124 = OpVectorShuffle %v3int %58 %58 0 1 2
%125 = OpIEqual %v3bool %123 %124
%126 = OpAll %bool %125
OpBranch %117
%117 = OpLabel
%127 = OpPhi %bool %false %105 %126 %116
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%131 = OpSelect %v4int %130 %58 %44
%132 = OpIEqual %v4bool %131 %58
%133 = OpAll %bool %132
OpBranch %129
%129 = OpLabel
%134 = OpPhi %bool %false %117 %133 %128
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%137 = OpIEqual %bool %int_0 %37
OpBranch %136
%136 = OpLabel
%138 = OpPhi %bool %false %129 %137 %135
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%143 = OpVectorShuffle %v2int %44 %44 0 1
%144 = OpIEqual %v2bool %142 %143
%145 = OpAll %bool %144
OpBranch %140
%140 = OpLabel
%146 = OpPhi %bool %false %136 %145 %139
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%150 = OpVectorShuffle %v3int %44 %44 0 1 2
%151 = OpIEqual %v3bool %149 %150
%152 = OpAll %bool %151
OpBranch %148
%148 = OpLabel
%153 = OpPhi %bool %false %140 %152 %147
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%157 = OpIEqual %v4bool %156 %44
%158 = OpAll %bool %157
OpBranch %155
%155 = OpLabel
%159 = OpPhi %bool %false %148 %158 %154
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpIEqual %bool %int_100 %51
OpBranch %161
%161 = OpLabel
%163 = OpPhi %bool %false %155 %162 %160
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%167 = OpVectorShuffle %v2int %58 %58 0 1
%168 = OpIEqual %v2bool %166 %167
%169 = OpAll %bool %168
OpBranch %165
%165 = OpLabel
%170 = OpPhi %bool %false %161 %169 %164
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%174 = OpVectorShuffle %v3int %58 %58 0 1 2
%175 = OpIEqual %v3bool %173 %174
%176 = OpAll %bool %175
OpBranch %172
%172 = OpLabel
%177 = OpPhi %bool %false %165 %176 %171
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%181 = OpIEqual %v4bool %180 %58
%182 = OpAll %bool %181
OpBranch %179
%179 = OpLabel
%183 = OpPhi %bool %false %172 %182 %178
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%187 = OpLoad %v4float %186
%188 = OpCompositeExtract %float %187 0
%189 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%190 = OpLoad %v4float %189
%191 = OpCompositeExtract %float %190 0
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%193 = OpLoad %v4float %192
%194 = OpCompositeExtract %float %193 0
%195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%196 = OpLoad %v4float %195
%197 = OpCompositeExtract %float %196 0
%198 = OpSelect %float %false %197 %194
%199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%200 = OpLoad %v4float %199
%201 = OpCompositeExtract %float %200 0
%202 = OpFOrdEqual %bool %198 %201
OpBranch %185
%185 = OpLabel
%203 = OpPhi %bool %false %179 %202 %184
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%207 = OpLoad %v4float %206
%208 = OpVectorShuffle %v2float %207 %207 0 1
%209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%210 = OpLoad %v4float %209
%211 = OpVectorShuffle %v2float %210 %210 0 1
%212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%213 = OpLoad %v4float %212
%214 = OpVectorShuffle %v2float %213 %213 0 1
%215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%216 = OpLoad %v4float %215
%217 = OpVectorShuffle %v2float %216 %216 0 1
%218 = OpSelect %v2float %68 %217 %214
%219 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%220 = OpLoad %v4float %219
%221 = OpVectorShuffle %v2float %220 %220 0 1
%222 = OpFOrdEqual %v2bool %218 %221
%223 = OpAll %bool %222
OpBranch %205
%205 = OpLabel
%224 = OpPhi %bool %false %185 %223 %204
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%227 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%228 = OpLoad %v4float %227
%229 = OpVectorShuffle %v3float %228 %228 0 1 2
%231 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%232 = OpLoad %v4float %231
%233 = OpVectorShuffle %v3float %232 %232 0 1 2
%234 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%235 = OpLoad %v4float %234
%236 = OpVectorShuffle %v3float %235 %235 0 1 2
%237 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%238 = OpLoad %v4float %237
%239 = OpVectorShuffle %v3float %238 %238 0 1 2
%240 = OpSelect %v3float %82 %239 %236
%241 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%242 = OpLoad %v4float %241
%243 = OpVectorShuffle %v3float %242 %242 0 1 2
%244 = OpFOrdEqual %v3bool %240 %243
%245 = OpAll %bool %244
OpBranch %226
%226 = OpLabel
%246 = OpPhi %bool %false %205 %245 %225
OpSelectionMerge %248 None
OpBranchConditional %246 %247 %248
%247 = OpLabel
%249 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%250 = OpLoad %v4float %249
%251 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%252 = OpLoad %v4float %251
%253 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%254 = OpLoad %v4float %253
%255 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%256 = OpLoad %v4float %255
%257 = OpSelect %v4float %93 %256 %254
%258 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%259 = OpLoad %v4float %258
%260 = OpFOrdEqual %v4bool %257 %259
%261 = OpAll %bool %260
OpBranch %248
%248 = OpLabel
%262 = OpPhi %bool %false %226 %261 %247
OpSelectionMerge %264 None
OpBranchConditional %262 %263 %264
%263 = OpLabel
%265 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%266 = OpLoad %v4float %265
%267 = OpCompositeExtract %float %266 0
%268 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%269 = OpLoad %v4float %268
%270 = OpCompositeExtract %float %269 0
%271 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%272 = OpLoad %v4float %271
%273 = OpCompositeExtract %float %272 0
%274 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%275 = OpLoad %v4float %274
%276 = OpCompositeExtract %float %275 0
%277 = OpSelect %float %true %276 %273
%278 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%279 = OpLoad %v4float %278
%280 = OpCompositeExtract %float %279 0
%281 = OpFOrdEqual %bool %277 %280
OpBranch %264
%264 = OpLabel
%282 = OpPhi %bool %false %248 %281 %263
OpSelectionMerge %284 None
OpBranchConditional %282 %283 %284
%283 = OpLabel
%285 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%286 = OpLoad %v4float %285
%287 = OpVectorShuffle %v2float %286 %286 0 1
%288 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%289 = OpLoad %v4float %288
%290 = OpVectorShuffle %v2float %289 %289 0 1
%291 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%292 = OpLoad %v4float %291
%293 = OpVectorShuffle %v2float %292 %292 0 1
%294 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%295 = OpLoad %v4float %294
%296 = OpVectorShuffle %v2float %295 %295 0 1
%297 = OpSelect %v2float %108 %296 %293
%298 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%299 = OpLoad %v4float %298
%300 = OpVectorShuffle %v2float %299 %299 0 1
%301 = OpFOrdEqual %v2bool %297 %300
%302 = OpAll %bool %301
OpBranch %284
%284 = OpLabel
%303 = OpPhi %bool %false %264 %302 %283
OpSelectionMerge %305 None
OpBranchConditional %303 %304 %305
%304 = OpLabel
%306 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%307 = OpLoad %v4float %306
%308 = OpVectorShuffle %v3float %307 %307 0 1 2
%309 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%310 = OpLoad %v4float %309
%311 = OpVectorShuffle %v3float %310 %310 0 1 2
%312 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%313 = OpLoad %v4float %312
%314 = OpVectorShuffle %v3float %313 %313 0 1 2
%315 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%316 = OpLoad %v4float %315
%317 = OpVectorShuffle %v3float %316 %316 0 1 2
%318 = OpSelect %v3float %120 %317 %314
%319 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%320 = OpLoad %v4float %319
%321 = OpVectorShuffle %v3float %320 %320 0 1 2
%322 = OpFOrdEqual %v3bool %318 %321
%323 = OpAll %bool %322
OpBranch %305
%305 = OpLabel
%324 = OpPhi %bool %false %284 %323 %304
OpSelectionMerge %326 None
OpBranchConditional %324 %325 %326
%325 = OpLabel
%327 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%328 = OpLoad %v4float %327
%329 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%330 = OpLoad %v4float %329
%331 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%332 = OpLoad %v4float %331
%333 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%334 = OpLoad %v4float %333
%335 = OpSelect %v4float %130 %334 %332
%336 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%337 = OpLoad %v4float %336
%338 = OpFOrdEqual %v4bool %335 %337
%339 = OpAll %bool %338
OpBranch %326
%326 = OpLabel
%340 = OpPhi %bool %false %305 %339 %325
OpSelectionMerge %342 None
OpBranchConditional %340 %341 %342
%341 = OpLabel
%343 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%344 = OpLoad %v4float %343
%345 = OpCompositeExtract %float %344 0
%346 = OpFOrdEqual %bool %float_0 %345
OpBranch %342
%342 = OpLabel
%347 = OpPhi %bool %false %326 %346 %341
OpSelectionMerge %349 None
OpBranchConditional %347 %348 %349
%348 = OpLabel
%352 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%353 = OpLoad %v4float %352
%354 = OpVectorShuffle %v2float %353 %353 0 1
%355 = OpFOrdEqual %v2bool %351 %354
%356 = OpAll %bool %355
OpBranch %349
%349 = OpLabel
%357 = OpPhi %bool %false %342 %356 %348
OpSelectionMerge %359 None
OpBranchConditional %357 %358 %359
%358 = OpLabel
%361 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%362 = OpLoad %v4float %361
%363 = OpVectorShuffle %v3float %362 %362 0 1 2
%364 = OpFOrdEqual %v3bool %360 %363
%365 = OpAll %bool %364
OpBranch %359
%359 = OpLabel
%366 = OpPhi %bool %false %349 %365 %358
OpSelectionMerge %368 None
OpBranchConditional %366 %367 %368
%367 = OpLabel
%370 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%371 = OpLoad %v4float %370
%372 = OpFOrdEqual %v4bool %369 %371
%373 = OpAll %bool %372
OpBranch %368
%368 = OpLabel
%374 = OpPhi %bool %false %359 %373 %367
OpSelectionMerge %376 None
OpBranchConditional %374 %375 %376
%375 = OpLabel
%377 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%378 = OpLoad %v4float %377
%379 = OpCompositeExtract %float %378 0
%380 = OpFOrdEqual %bool %float_1 %379
OpBranch %376
%376 = OpLabel
%381 = OpPhi %bool %false %368 %380 %375
OpSelectionMerge %383 None
OpBranchConditional %381 %382 %383
%382 = OpLabel
%385 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%386 = OpLoad %v4float %385
%387 = OpVectorShuffle %v2float %386 %386 0 1
%388 = OpFOrdEqual %v2bool %384 %387
%389 = OpAll %bool %388
OpBranch %383
%383 = OpLabel
%390 = OpPhi %bool %false %376 %389 %382
OpSelectionMerge %392 None
OpBranchConditional %390 %391 %392
%391 = OpLabel
%394 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%395 = OpLoad %v4float %394
%396 = OpVectorShuffle %v3float %395 %395 0 1 2
%397 = OpFOrdEqual %v3bool %393 %396
%398 = OpAll %bool %397
OpBranch %392
%392 = OpLabel
%399 = OpPhi %bool %false %383 %398 %391
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%403 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%404 = OpLoad %v4float %403
%405 = OpFOrdEqual %v4bool %402 %404
%406 = OpAll %bool %405
OpBranch %401
%401 = OpLabel
%407 = OpPhi %bool %false %392 %406 %400
OpSelectionMerge %412 None
OpBranchConditional %407 %410 %411
%410 = OpLabel
%413 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%414 = OpLoad %v4float %413
OpStore %408 %414
OpBranch %412
%411 = OpLabel
%415 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%416 = OpLoad %v4float %415
OpStore %408 %416
OpBranch %412
%412 = OpLabel
%417 = OpLoad %v4float %408
OpReturnValue %417
OpFunctionEnd
