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
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m1 "m1"
OpName %m3 "m3"
OpName %m4 "m4"
OpName %m5 "m5"
OpName %m7 "m7"
OpName %m9 "m9"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %test_comma_b "test_comma_b"
OpName %x "x"
OpName %y "y"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m1 "_1_m1"
OpName %_2_m3 "_2_m3"
OpName %_3_m4 "_3_m4"
OpName %_4_m5 "_4_m5"
OpName %_5_m7 "_5_m7"
OpName %_6_m9 "_6_m9"
OpName %_7_m10 "_7_m10"
OpName %_8_m11 "_8_m11"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %m1 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %m5 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %m7 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %455 RelaxedPrecision
OpDecorate %456 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%37 = OpConstantComposite %v2float %float_1 %float_2
%38 = OpConstantComposite %v2float %float_3 %float_4
%39 = OpConstantComposite %mat2v2float %37 %38
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_6 = OpConstant %float 6
%68 = OpConstantComposite %v2float %float_6 %float_0
%69 = OpConstantComposite %v2float %float_0 %float_6
%70 = OpConstantComposite %mat2v2float %68 %69
%float_12 = OpConstant %float 12
%float_18 = OpConstant %float 18
%float_24 = OpConstant %float 24
%91 = OpConstantComposite %v2float %float_6 %float_12
%92 = OpConstantComposite %v2float %float_18 %float_24
%93 = OpConstantComposite %mat2v2float %91 %92
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%114 = OpConstantComposite %v2float %float_4 %float_0
%115 = OpConstantComposite %v2float %float_0 %float_4
%116 = OpConstantComposite %mat2v2float %114 %115
%float_5 = OpConstant %float 5
%float_8 = OpConstant %float 8
%139 = OpConstantComposite %v2float %float_5 %float_2
%140 = OpConstantComposite %v2float %float_3 %float_8
%141 = OpConstantComposite %mat2v2float %139 %140
%float_7 = OpConstant %float 7
%152 = OpConstantComposite %v2float %float_5 %float_6
%153 = OpConstantComposite %v2float %float_7 %float_8
%154 = OpConstantComposite %mat2v2float %152 %153
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%171 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%172 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%173 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%174 = OpConstantComposite %mat3v3float %171 %172 %173
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%195 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%196 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%197 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%198 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%199 = OpConstantComposite %mat4v4float %195 %196 %197 %198
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%222 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%223 = OpConstantComposite %mat4v4float %222 %222 %222 %222
%237 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%238 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%239 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%240 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%241 = OpConstantComposite %mat4v4float %237 %238 %239 %240
%266 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %25
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m1 = OpVariable %_ptr_Function_mat2v2float Function
%m3 = OpVariable %_ptr_Function_mat2v2float Function
%m4 = OpVariable %_ptr_Function_mat2v2float Function
%m5 = OpVariable %_ptr_Function_mat2v2float Function
%m7 = OpVariable %_ptr_Function_mat2v2float Function
%m9 = OpVariable %_ptr_Function_mat3v3float Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
OpStore %m1 %39
OpSelectionMerge %42 None
OpBranchConditional %true %41 %42
%41 = OpLabel
%43 = OpLoad %mat2v2float %m1
%45 = OpCompositeExtract %v2float %43 0
%46 = OpFOrdEqual %v2bool %45 %37
%47 = OpAll %bool %46
%48 = OpCompositeExtract %v2float %43 1
%49 = OpFOrdEqual %v2bool %48 %38
%50 = OpAll %bool %49
%51 = OpLogicalAnd %bool %47 %50
OpBranch %42
%42 = OpLabel
%52 = OpPhi %bool %false %26 %51 %41
OpStore %ok %52
%54 = OpLoad %mat2v2float %m1
OpStore %m3 %54
OpSelectionMerge %56 None
OpBranchConditional %52 %55 %56
%55 = OpLabel
%57 = OpLoad %mat2v2float %m3
%58 = OpCompositeExtract %v2float %57 0
%59 = OpFOrdEqual %v2bool %58 %37
%60 = OpAll %bool %59
%61 = OpCompositeExtract %v2float %57 1
%62 = OpFOrdEqual %v2bool %61 %38
%63 = OpAll %bool %62
%64 = OpLogicalAnd %bool %60 %63
OpBranch %56
%56 = OpLabel
%65 = OpPhi %bool %false %42 %64 %55
OpStore %ok %65
OpStore %m4 %70
OpSelectionMerge %72 None
OpBranchConditional %65 %71 %72
%71 = OpLabel
%73 = OpLoad %mat2v2float %m4
%74 = OpCompositeExtract %v2float %73 0
%75 = OpFOrdEqual %v2bool %74 %68
%76 = OpAll %bool %75
%77 = OpCompositeExtract %v2float %73 1
%78 = OpFOrdEqual %v2bool %77 %69
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %76 %79
OpBranch %72
%72 = OpLabel
%81 = OpPhi %bool %false %56 %80 %71
OpStore %ok %81
%82 = OpLoad %mat2v2float %m3
%83 = OpLoad %mat2v2float %m4
%84 = OpMatrixTimesMatrix %mat2v2float %82 %83
OpStore %m3 %84
OpSelectionMerge %86 None
OpBranchConditional %81 %85 %86
%85 = OpLabel
%87 = OpLoad %mat2v2float %m3
%94 = OpCompositeExtract %v2float %87 0
%95 = OpFOrdEqual %v2bool %94 %91
%96 = OpAll %bool %95
%97 = OpCompositeExtract %v2float %87 1
%98 = OpFOrdEqual %v2bool %97 %92
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %96 %99
OpBranch %86
%86 = OpLabel
%101 = OpPhi %bool %false %72 %100 %85
OpStore %ok %101
%105 = OpAccessChain %_ptr_Function_v2float %m1 %int_1
%106 = OpLoad %v2float %105
%107 = OpCompositeExtract %float %106 1
%108 = OpCompositeConstruct %v2float %107 %float_0
%109 = OpCompositeConstruct %v2float %float_0 %107
%110 = OpCompositeConstruct %mat2v2float %108 %109
OpStore %m5 %110
OpSelectionMerge %112 None
OpBranchConditional %101 %111 %112
%111 = OpLabel
%113 = OpLoad %mat2v2float %m5
%117 = OpCompositeExtract %v2float %113 0
%118 = OpFOrdEqual %v2bool %117 %114
%119 = OpAll %bool %118
%120 = OpCompositeExtract %v2float %113 1
%121 = OpFOrdEqual %v2bool %120 %115
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %119 %122
OpBranch %112
%112 = OpLabel
%124 = OpPhi %bool %false %86 %123 %111
OpStore %ok %124
%125 = OpLoad %mat2v2float %m1
%126 = OpLoad %mat2v2float %m5
%127 = OpCompositeExtract %v2float %125 0
%128 = OpCompositeExtract %v2float %126 0
%129 = OpFAdd %v2float %127 %128
%130 = OpCompositeExtract %v2float %125 1
%131 = OpCompositeExtract %v2float %126 1
%132 = OpFAdd %v2float %130 %131
%133 = OpCompositeConstruct %mat2v2float %129 %132
OpStore %m1 %133
OpSelectionMerge %135 None
OpBranchConditional %124 %134 %135
%134 = OpLabel
%136 = OpLoad %mat2v2float %m1
%142 = OpCompositeExtract %v2float %136 0
%143 = OpFOrdEqual %v2bool %142 %139
%144 = OpAll %bool %143
%145 = OpCompositeExtract %v2float %136 1
%146 = OpFOrdEqual %v2bool %145 %140
%147 = OpAll %bool %146
%148 = OpLogicalAnd %bool %144 %147
OpBranch %135
%135 = OpLabel
%149 = OpPhi %bool %false %112 %148 %134
OpStore %ok %149
OpStore %m7 %154
OpSelectionMerge %156 None
OpBranchConditional %149 %155 %156
%155 = OpLabel
%157 = OpLoad %mat2v2float %m7
%158 = OpCompositeExtract %v2float %157 0
%159 = OpFOrdEqual %v2bool %158 %152
%160 = OpAll %bool %159
%161 = OpCompositeExtract %v2float %157 1
%162 = OpFOrdEqual %v2bool %161 %153
%163 = OpAll %bool %162
%164 = OpLogicalAnd %bool %160 %163
OpBranch %156
%156 = OpLabel
%165 = OpPhi %bool %false %135 %164 %155
OpStore %ok %165
OpStore %m9 %174
OpSelectionMerge %176 None
OpBranchConditional %165 %175 %176
%175 = OpLabel
%177 = OpLoad %mat3v3float %m9
%179 = OpCompositeExtract %v3float %177 0
%180 = OpFOrdEqual %v3bool %179 %171
%181 = OpAll %bool %180
%182 = OpCompositeExtract %v3float %177 1
%183 = OpFOrdEqual %v3bool %182 %172
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %181 %184
%186 = OpCompositeExtract %v3float %177 2
%187 = OpFOrdEqual %v3bool %186 %173
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %185 %188
OpBranch %176
%176 = OpLabel
%190 = OpPhi %bool %false %156 %189 %175
OpStore %ok %190
OpStore %m10 %199
OpSelectionMerge %201 None
OpBranchConditional %190 %200 %201
%200 = OpLabel
%202 = OpLoad %mat4v4float %m10
%204 = OpCompositeExtract %v4float %202 0
%205 = OpFOrdEqual %v4bool %204 %195
%206 = OpAll %bool %205
%207 = OpCompositeExtract %v4float %202 1
%208 = OpFOrdEqual %v4bool %207 %196
%209 = OpAll %bool %208
%210 = OpLogicalAnd %bool %206 %209
%211 = OpCompositeExtract %v4float %202 2
%212 = OpFOrdEqual %v4bool %211 %197
%213 = OpAll %bool %212
%214 = OpLogicalAnd %bool %210 %213
%215 = OpCompositeExtract %v4float %202 3
%216 = OpFOrdEqual %v4bool %215 %198
%217 = OpAll %bool %216
%218 = OpLogicalAnd %bool %214 %217
OpBranch %201
%201 = OpLabel
%219 = OpPhi %bool %false %176 %218 %200
OpStore %ok %219
OpStore %m11 %223
%224 = OpLoad %mat4v4float %m10
%225 = OpCompositeExtract %v4float %224 0
%226 = OpFSub %v4float %222 %225
%227 = OpCompositeExtract %v4float %224 1
%228 = OpFSub %v4float %222 %227
%229 = OpCompositeExtract %v4float %224 2
%230 = OpFSub %v4float %222 %229
%231 = OpCompositeExtract %v4float %224 3
%232 = OpFSub %v4float %222 %231
%233 = OpCompositeConstruct %mat4v4float %226 %228 %230 %232
OpStore %m11 %233
OpSelectionMerge %235 None
OpBranchConditional %219 %234 %235
%234 = OpLabel
%236 = OpLoad %mat4v4float %m11
%242 = OpCompositeExtract %v4float %236 0
%243 = OpFOrdEqual %v4bool %242 %237
%244 = OpAll %bool %243
%245 = OpCompositeExtract %v4float %236 1
%246 = OpFOrdEqual %v4bool %245 %238
%247 = OpAll %bool %246
%248 = OpLogicalAnd %bool %244 %247
%249 = OpCompositeExtract %v4float %236 2
%250 = OpFOrdEqual %v4bool %249 %239
%251 = OpAll %bool %250
%252 = OpLogicalAnd %bool %248 %251
%253 = OpCompositeExtract %v4float %236 3
%254 = OpFOrdEqual %v4bool %253 %240
%255 = OpAll %bool %254
%256 = OpLogicalAnd %bool %252 %255
OpBranch %235
%235 = OpLabel
%257 = OpPhi %bool %false %201 %256 %234
OpStore %ok %257
OpReturnValue %257
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%258 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
OpStore %x %39
OpStore %y %39
%261 = OpFOrdEqual %v2bool %37 %37
%262 = OpAll %bool %261
%263 = OpFOrdEqual %v2bool %38 %38
%264 = OpAll %bool %263
%265 = OpLogicalAnd %bool %262 %264
OpReturnValue %265
OpFunctionEnd
%main = OpFunction %v4float None %266
%267 = OpFunctionParameter %_ptr_Function_v2float
%268 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%445 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m1 %39
OpSelectionMerge %272 None
OpBranchConditional %true %271 %272
%271 = OpLabel
%273 = OpLoad %mat2v2float %_1_m1
%274 = OpCompositeExtract %v2float %273 0
%275 = OpFOrdEqual %v2bool %274 %37
%276 = OpAll %bool %275
%277 = OpCompositeExtract %v2float %273 1
%278 = OpFOrdEqual %v2bool %277 %38
%279 = OpAll %bool %278
%280 = OpLogicalAnd %bool %276 %279
OpBranch %272
%272 = OpLabel
%281 = OpPhi %bool %false %268 %280 %271
OpStore %_0_ok %281
%283 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %283
OpSelectionMerge %285 None
OpBranchConditional %281 %284 %285
%284 = OpLabel
%286 = OpLoad %mat2v2float %_2_m3
%287 = OpCompositeExtract %v2float %286 0
%288 = OpFOrdEqual %v2bool %287 %37
%289 = OpAll %bool %288
%290 = OpCompositeExtract %v2float %286 1
%291 = OpFOrdEqual %v2bool %290 %38
%292 = OpAll %bool %291
%293 = OpLogicalAnd %bool %289 %292
OpBranch %285
%285 = OpLabel
%294 = OpPhi %bool %false %272 %293 %284
OpStore %_0_ok %294
OpStore %_3_m4 %70
OpSelectionMerge %297 None
OpBranchConditional %294 %296 %297
%296 = OpLabel
%298 = OpLoad %mat2v2float %_3_m4
%299 = OpCompositeExtract %v2float %298 0
%300 = OpFOrdEqual %v2bool %299 %68
%301 = OpAll %bool %300
%302 = OpCompositeExtract %v2float %298 1
%303 = OpFOrdEqual %v2bool %302 %69
%304 = OpAll %bool %303
%305 = OpLogicalAnd %bool %301 %304
OpBranch %297
%297 = OpLabel
%306 = OpPhi %bool %false %285 %305 %296
OpStore %_0_ok %306
%307 = OpLoad %mat2v2float %_2_m3
%308 = OpLoad %mat2v2float %_3_m4
%309 = OpMatrixTimesMatrix %mat2v2float %307 %308
OpStore %_2_m3 %309
OpSelectionMerge %311 None
OpBranchConditional %306 %310 %311
%310 = OpLabel
%312 = OpLoad %mat2v2float %_2_m3
%313 = OpCompositeExtract %v2float %312 0
%314 = OpFOrdEqual %v2bool %313 %91
%315 = OpAll %bool %314
%316 = OpCompositeExtract %v2float %312 1
%317 = OpFOrdEqual %v2bool %316 %92
%318 = OpAll %bool %317
%319 = OpLogicalAnd %bool %315 %318
OpBranch %311
%311 = OpLabel
%320 = OpPhi %bool %false %297 %319 %310
OpStore %_0_ok %320
%322 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%323 = OpLoad %v2float %322
%324 = OpCompositeExtract %float %323 1
%325 = OpCompositeConstruct %v2float %324 %float_0
%326 = OpCompositeConstruct %v2float %float_0 %324
%327 = OpCompositeConstruct %mat2v2float %325 %326
OpStore %_4_m5 %327
OpSelectionMerge %329 None
OpBranchConditional %320 %328 %329
%328 = OpLabel
%330 = OpLoad %mat2v2float %_4_m5
%331 = OpCompositeExtract %v2float %330 0
%332 = OpFOrdEqual %v2bool %331 %114
%333 = OpAll %bool %332
%334 = OpCompositeExtract %v2float %330 1
%335 = OpFOrdEqual %v2bool %334 %115
%336 = OpAll %bool %335
%337 = OpLogicalAnd %bool %333 %336
OpBranch %329
%329 = OpLabel
%338 = OpPhi %bool %false %311 %337 %328
OpStore %_0_ok %338
%339 = OpLoad %mat2v2float %_1_m1
%340 = OpLoad %mat2v2float %_4_m5
%341 = OpCompositeExtract %v2float %339 0
%342 = OpCompositeExtract %v2float %340 0
%343 = OpFAdd %v2float %341 %342
%344 = OpCompositeExtract %v2float %339 1
%345 = OpCompositeExtract %v2float %340 1
%346 = OpFAdd %v2float %344 %345
%347 = OpCompositeConstruct %mat2v2float %343 %346
OpStore %_1_m1 %347
OpSelectionMerge %349 None
OpBranchConditional %338 %348 %349
%348 = OpLabel
%350 = OpLoad %mat2v2float %_1_m1
%351 = OpCompositeExtract %v2float %350 0
%352 = OpFOrdEqual %v2bool %351 %139
%353 = OpAll %bool %352
%354 = OpCompositeExtract %v2float %350 1
%355 = OpFOrdEqual %v2bool %354 %140
%356 = OpAll %bool %355
%357 = OpLogicalAnd %bool %353 %356
OpBranch %349
%349 = OpLabel
%358 = OpPhi %bool %false %329 %357 %348
OpStore %_0_ok %358
OpStore %_5_m7 %154
OpSelectionMerge %361 None
OpBranchConditional %358 %360 %361
%360 = OpLabel
%362 = OpLoad %mat2v2float %_5_m7
%363 = OpCompositeExtract %v2float %362 0
%364 = OpFOrdEqual %v2bool %363 %152
%365 = OpAll %bool %364
%366 = OpCompositeExtract %v2float %362 1
%367 = OpFOrdEqual %v2bool %366 %153
%368 = OpAll %bool %367
%369 = OpLogicalAnd %bool %365 %368
OpBranch %361
%361 = OpLabel
%370 = OpPhi %bool %false %349 %369 %360
OpStore %_0_ok %370
OpStore %_6_m9 %174
OpSelectionMerge %373 None
OpBranchConditional %370 %372 %373
%372 = OpLabel
%374 = OpLoad %mat3v3float %_6_m9
%375 = OpCompositeExtract %v3float %374 0
%376 = OpFOrdEqual %v3bool %375 %171
%377 = OpAll %bool %376
%378 = OpCompositeExtract %v3float %374 1
%379 = OpFOrdEqual %v3bool %378 %172
%380 = OpAll %bool %379
%381 = OpLogicalAnd %bool %377 %380
%382 = OpCompositeExtract %v3float %374 2
%383 = OpFOrdEqual %v3bool %382 %173
%384 = OpAll %bool %383
%385 = OpLogicalAnd %bool %381 %384
OpBranch %373
%373 = OpLabel
%386 = OpPhi %bool %false %361 %385 %372
OpStore %_0_ok %386
OpStore %_7_m10 %199
OpSelectionMerge %389 None
OpBranchConditional %386 %388 %389
%388 = OpLabel
%390 = OpLoad %mat4v4float %_7_m10
%391 = OpCompositeExtract %v4float %390 0
%392 = OpFOrdEqual %v4bool %391 %195
%393 = OpAll %bool %392
%394 = OpCompositeExtract %v4float %390 1
%395 = OpFOrdEqual %v4bool %394 %196
%396 = OpAll %bool %395
%397 = OpLogicalAnd %bool %393 %396
%398 = OpCompositeExtract %v4float %390 2
%399 = OpFOrdEqual %v4bool %398 %197
%400 = OpAll %bool %399
%401 = OpLogicalAnd %bool %397 %400
%402 = OpCompositeExtract %v4float %390 3
%403 = OpFOrdEqual %v4bool %402 %198
%404 = OpAll %bool %403
%405 = OpLogicalAnd %bool %401 %404
OpBranch %389
%389 = OpLabel
%406 = OpPhi %bool %false %373 %405 %388
OpStore %_0_ok %406
OpStore %_8_m11 %223
%408 = OpLoad %mat4v4float %_7_m10
%409 = OpCompositeExtract %v4float %408 0
%410 = OpFSub %v4float %222 %409
%411 = OpCompositeExtract %v4float %408 1
%412 = OpFSub %v4float %222 %411
%413 = OpCompositeExtract %v4float %408 2
%414 = OpFSub %v4float %222 %413
%415 = OpCompositeExtract %v4float %408 3
%416 = OpFSub %v4float %222 %415
%417 = OpCompositeConstruct %mat4v4float %410 %412 %414 %416
OpStore %_8_m11 %417
OpSelectionMerge %419 None
OpBranchConditional %406 %418 %419
%418 = OpLabel
%420 = OpLoad %mat4v4float %_8_m11
%421 = OpCompositeExtract %v4float %420 0
%422 = OpFOrdEqual %v4bool %421 %237
%423 = OpAll %bool %422
%424 = OpCompositeExtract %v4float %420 1
%425 = OpFOrdEqual %v4bool %424 %238
%426 = OpAll %bool %425
%427 = OpLogicalAnd %bool %423 %426
%428 = OpCompositeExtract %v4float %420 2
%429 = OpFOrdEqual %v4bool %428 %239
%430 = OpAll %bool %429
%431 = OpLogicalAnd %bool %427 %430
%432 = OpCompositeExtract %v4float %420 3
%433 = OpFOrdEqual %v4bool %432 %240
%434 = OpAll %bool %433
%435 = OpLogicalAnd %bool %431 %434
OpBranch %419
%419 = OpLabel
%436 = OpPhi %bool %false %389 %435 %418
OpStore %_0_ok %436
OpSelectionMerge %438 None
OpBranchConditional %436 %437 %438
%437 = OpLabel
%439 = OpFunctionCall %bool %test_half_b
OpBranch %438
%438 = OpLabel
%440 = OpPhi %bool %false %419 %439 %437
OpSelectionMerge %442 None
OpBranchConditional %440 %441 %442
%441 = OpLabel
%443 = OpFunctionCall %bool %test_comma_b
OpBranch %442
%442 = OpLabel
%444 = OpPhi %bool %false %438 %443 %441
OpSelectionMerge %449 None
OpBranchConditional %444 %447 %448
%447 = OpLabel
%450 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%453 = OpLoad %v4float %450
OpStore %445 %453
OpBranch %449
%448 = OpLabel
%454 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%455 = OpLoad %v4float %454
OpStore %445 %455
OpBranch %449
%449 = OpLabel
%456 = OpLoad %v4float %445
OpReturnValue %456
OpFunctionEnd
