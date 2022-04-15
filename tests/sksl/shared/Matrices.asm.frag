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
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %m3 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %m4 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
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
OpDecorate %m7 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %m9 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %m10 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
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
OpDecorate %234 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %447 RelaxedPrecision
OpDecorate %449 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
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
%69 = OpConstantComposite %v2float %float_6 %float_0
%70 = OpConstantComposite %v2float %float_0 %float_6
%71 = OpConstantComposite %mat2v2float %69 %70
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
%138 = OpConstantComposite %v2float %float_5 %float_2
%139 = OpConstantComposite %v2float %float_3 %float_8
%140 = OpConstantComposite %mat2v2float %138 %139
%float_7 = OpConstant %float 7
%149 = OpConstantComposite %v2float %float_5 %float_6
%150 = OpConstantComposite %v2float %float_7 %float_8
%151 = OpConstantComposite %mat2v2float %149 %150
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_9 = OpConstant %float 9
%168 = OpConstantComposite %v3float %float_9 %float_0 %float_0
%169 = OpConstantComposite %v3float %float_0 %float_9 %float_0
%170 = OpConstantComposite %v3float %float_0 %float_0 %float_9
%171 = OpConstantComposite %mat3v3float %168 %169 %170
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%192 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
%193 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
%194 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
%195 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
%196 = OpConstantComposite %mat4v4float %192 %193 %194 %195
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%219 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
%220 = OpConstantComposite %mat4v4float %219 %219 %219 %219
%238 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
%239 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
%240 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
%241 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
%242 = OpConstantComposite %mat4v4float %238 %239 %240 %241
%263 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%41 = OpLoad %bool %ok
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %mat2v2float %m1
%46 = OpCompositeExtract %v2float %44 0
%47 = OpFOrdEqual %v2bool %46 %37
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v2float %44 1
%50 = OpFOrdEqual %v2bool %49 %38
%51 = OpAll %bool %50
%52 = OpLogicalAnd %bool %48 %51
OpBranch %43
%43 = OpLabel
%53 = OpPhi %bool %false %26 %52 %42
OpStore %ok %53
%55 = OpLoad %mat2v2float %m1
OpStore %m3 %55
OpSelectionMerge %57 None
OpBranchConditional %53 %56 %57
%56 = OpLabel
%58 = OpLoad %mat2v2float %m3
%59 = OpCompositeExtract %v2float %58 0
%60 = OpFOrdEqual %v2bool %59 %37
%61 = OpAll %bool %60
%62 = OpCompositeExtract %v2float %58 1
%63 = OpFOrdEqual %v2bool %62 %38
%64 = OpAll %bool %63
%65 = OpLogicalAnd %bool %61 %64
OpBranch %57
%57 = OpLabel
%66 = OpPhi %bool %false %43 %65 %56
OpStore %ok %66
OpStore %m4 %71
OpSelectionMerge %73 None
OpBranchConditional %66 %72 %73
%72 = OpLabel
%74 = OpLoad %mat2v2float %m4
%75 = OpCompositeExtract %v2float %74 0
%76 = OpFOrdEqual %v2bool %75 %69
%77 = OpAll %bool %76
%78 = OpCompositeExtract %v2float %74 1
%79 = OpFOrdEqual %v2bool %78 %70
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %77 %80
OpBranch %73
%73 = OpLabel
%82 = OpPhi %bool %false %57 %81 %72
OpStore %ok %82
%83 = OpLoad %mat2v2float %m3
%84 = OpLoad %mat2v2float %m4
%85 = OpMatrixTimesMatrix %mat2v2float %83 %84
OpStore %m3 %85
OpSelectionMerge %87 None
OpBranchConditional %82 %86 %87
%86 = OpLabel
%94 = OpCompositeExtract %v2float %85 0
%95 = OpFOrdEqual %v2bool %94 %91
%96 = OpAll %bool %95
%97 = OpCompositeExtract %v2float %85 1
%98 = OpFOrdEqual %v2bool %97 %92
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %96 %99
OpBranch %87
%87 = OpLabel
%101 = OpPhi %bool %false %73 %100 %86
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
%124 = OpPhi %bool %false %87 %123 %111
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
%141 = OpFOrdEqual %v2bool %129 %138
%142 = OpAll %bool %141
%143 = OpFOrdEqual %v2bool %132 %139
%144 = OpAll %bool %143
%145 = OpLogicalAnd %bool %142 %144
OpBranch %135
%135 = OpLabel
%146 = OpPhi %bool %false %112 %145 %134
OpStore %ok %146
OpStore %m7 %151
OpSelectionMerge %153 None
OpBranchConditional %146 %152 %153
%152 = OpLabel
%154 = OpLoad %mat2v2float %m7
%155 = OpCompositeExtract %v2float %154 0
%156 = OpFOrdEqual %v2bool %155 %149
%157 = OpAll %bool %156
%158 = OpCompositeExtract %v2float %154 1
%159 = OpFOrdEqual %v2bool %158 %150
%160 = OpAll %bool %159
%161 = OpLogicalAnd %bool %157 %160
OpBranch %153
%153 = OpLabel
%162 = OpPhi %bool %false %135 %161 %152
OpStore %ok %162
OpStore %m9 %171
OpSelectionMerge %173 None
OpBranchConditional %162 %172 %173
%172 = OpLabel
%174 = OpLoad %mat3v3float %m9
%176 = OpCompositeExtract %v3float %174 0
%177 = OpFOrdEqual %v3bool %176 %168
%178 = OpAll %bool %177
%179 = OpCompositeExtract %v3float %174 1
%180 = OpFOrdEqual %v3bool %179 %169
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %178 %181
%183 = OpCompositeExtract %v3float %174 2
%184 = OpFOrdEqual %v3bool %183 %170
%185 = OpAll %bool %184
%186 = OpLogicalAnd %bool %182 %185
OpBranch %173
%173 = OpLabel
%187 = OpPhi %bool %false %153 %186 %172
OpStore %ok %187
OpStore %m10 %196
OpSelectionMerge %198 None
OpBranchConditional %187 %197 %198
%197 = OpLabel
%199 = OpLoad %mat4v4float %m10
%201 = OpCompositeExtract %v4float %199 0
%202 = OpFOrdEqual %v4bool %201 %192
%203 = OpAll %bool %202
%204 = OpCompositeExtract %v4float %199 1
%205 = OpFOrdEqual %v4bool %204 %193
%206 = OpAll %bool %205
%207 = OpLogicalAnd %bool %203 %206
%208 = OpCompositeExtract %v4float %199 2
%209 = OpFOrdEqual %v4bool %208 %194
%210 = OpAll %bool %209
%211 = OpLogicalAnd %bool %207 %210
%212 = OpCompositeExtract %v4float %199 3
%213 = OpFOrdEqual %v4bool %212 %195
%214 = OpAll %bool %213
%215 = OpLogicalAnd %bool %211 %214
OpBranch %198
%198 = OpLabel
%216 = OpPhi %bool %false %173 %215 %197
OpStore %ok %216
OpStore %m11 %220
%221 = OpLoad %mat4v4float %m11
%222 = OpLoad %mat4v4float %m10
%223 = OpCompositeExtract %v4float %221 0
%224 = OpCompositeExtract %v4float %222 0
%225 = OpFSub %v4float %223 %224
%226 = OpCompositeExtract %v4float %221 1
%227 = OpCompositeExtract %v4float %222 1
%228 = OpFSub %v4float %226 %227
%229 = OpCompositeExtract %v4float %221 2
%230 = OpCompositeExtract %v4float %222 2
%231 = OpFSub %v4float %229 %230
%232 = OpCompositeExtract %v4float %221 3
%233 = OpCompositeExtract %v4float %222 3
%234 = OpFSub %v4float %232 %233
%235 = OpCompositeConstruct %mat4v4float %225 %228 %231 %234
OpStore %m11 %235
OpSelectionMerge %237 None
OpBranchConditional %216 %236 %237
%236 = OpLabel
%243 = OpFOrdEqual %v4bool %225 %238
%244 = OpAll %bool %243
%245 = OpFOrdEqual %v4bool %228 %239
%246 = OpAll %bool %245
%247 = OpLogicalAnd %bool %244 %246
%248 = OpFOrdEqual %v4bool %231 %240
%249 = OpAll %bool %248
%250 = OpLogicalAnd %bool %247 %249
%251 = OpFOrdEqual %v4bool %234 %241
%252 = OpAll %bool %251
%253 = OpLogicalAnd %bool %250 %252
OpBranch %237
%237 = OpLabel
%254 = OpPhi %bool %false %198 %253 %236
OpStore %ok %254
OpReturnValue %254
OpFunctionEnd
%test_comma_b = OpFunction %bool None %25
%255 = OpLabel
%x = OpVariable %_ptr_Function_mat2v2float Function
%y = OpVariable %_ptr_Function_mat2v2float Function
OpStore %x %39
OpStore %y %39
%258 = OpFOrdEqual %v2bool %37 %37
%259 = OpAll %bool %258
%260 = OpFOrdEqual %v2bool %38 %38
%261 = OpAll %bool %260
%262 = OpLogicalAnd %bool %259 %261
OpReturnValue %262
OpFunctionEnd
%main = OpFunction %v4float None %263
%264 = OpFunctionParameter %_ptr_Function_v2float
%265 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m1 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m4 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m5 = OpVariable %_ptr_Function_mat2v2float Function
%_5_m7 = OpVariable %_ptr_Function_mat2v2float Function
%_6_m9 = OpVariable %_ptr_Function_mat3v3float Function
%_7_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_8_m11 = OpVariable %_ptr_Function_mat4v4float Function
%439 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
OpStore %_1_m1 %39
%268 = OpLoad %bool %_0_ok
OpSelectionMerge %270 None
OpBranchConditional %268 %269 %270
%269 = OpLabel
%271 = OpLoad %mat2v2float %_1_m1
%272 = OpCompositeExtract %v2float %271 0
%273 = OpFOrdEqual %v2bool %272 %37
%274 = OpAll %bool %273
%275 = OpCompositeExtract %v2float %271 1
%276 = OpFOrdEqual %v2bool %275 %38
%277 = OpAll %bool %276
%278 = OpLogicalAnd %bool %274 %277
OpBranch %270
%270 = OpLabel
%279 = OpPhi %bool %false %265 %278 %269
OpStore %_0_ok %279
%281 = OpLoad %mat2v2float %_1_m1
OpStore %_2_m3 %281
OpSelectionMerge %283 None
OpBranchConditional %279 %282 %283
%282 = OpLabel
%284 = OpLoad %mat2v2float %_2_m3
%285 = OpCompositeExtract %v2float %284 0
%286 = OpFOrdEqual %v2bool %285 %37
%287 = OpAll %bool %286
%288 = OpCompositeExtract %v2float %284 1
%289 = OpFOrdEqual %v2bool %288 %38
%290 = OpAll %bool %289
%291 = OpLogicalAnd %bool %287 %290
OpBranch %283
%283 = OpLabel
%292 = OpPhi %bool %false %270 %291 %282
OpStore %_0_ok %292
OpStore %_3_m4 %71
OpSelectionMerge %295 None
OpBranchConditional %292 %294 %295
%294 = OpLabel
%296 = OpLoad %mat2v2float %_3_m4
%297 = OpCompositeExtract %v2float %296 0
%298 = OpFOrdEqual %v2bool %297 %69
%299 = OpAll %bool %298
%300 = OpCompositeExtract %v2float %296 1
%301 = OpFOrdEqual %v2bool %300 %70
%302 = OpAll %bool %301
%303 = OpLogicalAnd %bool %299 %302
OpBranch %295
%295 = OpLabel
%304 = OpPhi %bool %false %283 %303 %294
OpStore %_0_ok %304
%305 = OpLoad %mat2v2float %_2_m3
%306 = OpLoad %mat2v2float %_3_m4
%307 = OpMatrixTimesMatrix %mat2v2float %305 %306
OpStore %_2_m3 %307
OpSelectionMerge %309 None
OpBranchConditional %304 %308 %309
%308 = OpLabel
%310 = OpCompositeExtract %v2float %307 0
%311 = OpFOrdEqual %v2bool %310 %91
%312 = OpAll %bool %311
%313 = OpCompositeExtract %v2float %307 1
%314 = OpFOrdEqual %v2bool %313 %92
%315 = OpAll %bool %314
%316 = OpLogicalAnd %bool %312 %315
OpBranch %309
%309 = OpLabel
%317 = OpPhi %bool %false %295 %316 %308
OpStore %_0_ok %317
%319 = OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
%320 = OpLoad %v2float %319
%321 = OpCompositeExtract %float %320 1
%322 = OpCompositeConstruct %v2float %321 %float_0
%323 = OpCompositeConstruct %v2float %float_0 %321
%324 = OpCompositeConstruct %mat2v2float %322 %323
OpStore %_4_m5 %324
OpSelectionMerge %326 None
OpBranchConditional %317 %325 %326
%325 = OpLabel
%327 = OpLoad %mat2v2float %_4_m5
%328 = OpCompositeExtract %v2float %327 0
%329 = OpFOrdEqual %v2bool %328 %114
%330 = OpAll %bool %329
%331 = OpCompositeExtract %v2float %327 1
%332 = OpFOrdEqual %v2bool %331 %115
%333 = OpAll %bool %332
%334 = OpLogicalAnd %bool %330 %333
OpBranch %326
%326 = OpLabel
%335 = OpPhi %bool %false %309 %334 %325
OpStore %_0_ok %335
%336 = OpLoad %mat2v2float %_1_m1
%337 = OpLoad %mat2v2float %_4_m5
%338 = OpCompositeExtract %v2float %336 0
%339 = OpCompositeExtract %v2float %337 0
%340 = OpFAdd %v2float %338 %339
%341 = OpCompositeExtract %v2float %336 1
%342 = OpCompositeExtract %v2float %337 1
%343 = OpFAdd %v2float %341 %342
%344 = OpCompositeConstruct %mat2v2float %340 %343
OpStore %_1_m1 %344
OpSelectionMerge %346 None
OpBranchConditional %335 %345 %346
%345 = OpLabel
%347 = OpFOrdEqual %v2bool %340 %138
%348 = OpAll %bool %347
%349 = OpFOrdEqual %v2bool %343 %139
%350 = OpAll %bool %349
%351 = OpLogicalAnd %bool %348 %350
OpBranch %346
%346 = OpLabel
%352 = OpPhi %bool %false %326 %351 %345
OpStore %_0_ok %352
OpStore %_5_m7 %151
OpSelectionMerge %355 None
OpBranchConditional %352 %354 %355
%354 = OpLabel
%356 = OpLoad %mat2v2float %_5_m7
%357 = OpCompositeExtract %v2float %356 0
%358 = OpFOrdEqual %v2bool %357 %149
%359 = OpAll %bool %358
%360 = OpCompositeExtract %v2float %356 1
%361 = OpFOrdEqual %v2bool %360 %150
%362 = OpAll %bool %361
%363 = OpLogicalAnd %bool %359 %362
OpBranch %355
%355 = OpLabel
%364 = OpPhi %bool %false %346 %363 %354
OpStore %_0_ok %364
OpStore %_6_m9 %171
OpSelectionMerge %367 None
OpBranchConditional %364 %366 %367
%366 = OpLabel
%368 = OpLoad %mat3v3float %_6_m9
%369 = OpCompositeExtract %v3float %368 0
%370 = OpFOrdEqual %v3bool %369 %168
%371 = OpAll %bool %370
%372 = OpCompositeExtract %v3float %368 1
%373 = OpFOrdEqual %v3bool %372 %169
%374 = OpAll %bool %373
%375 = OpLogicalAnd %bool %371 %374
%376 = OpCompositeExtract %v3float %368 2
%377 = OpFOrdEqual %v3bool %376 %170
%378 = OpAll %bool %377
%379 = OpLogicalAnd %bool %375 %378
OpBranch %367
%367 = OpLabel
%380 = OpPhi %bool %false %355 %379 %366
OpStore %_0_ok %380
OpStore %_7_m10 %196
OpSelectionMerge %383 None
OpBranchConditional %380 %382 %383
%382 = OpLabel
%384 = OpLoad %mat4v4float %_7_m10
%385 = OpCompositeExtract %v4float %384 0
%386 = OpFOrdEqual %v4bool %385 %192
%387 = OpAll %bool %386
%388 = OpCompositeExtract %v4float %384 1
%389 = OpFOrdEqual %v4bool %388 %193
%390 = OpAll %bool %389
%391 = OpLogicalAnd %bool %387 %390
%392 = OpCompositeExtract %v4float %384 2
%393 = OpFOrdEqual %v4bool %392 %194
%394 = OpAll %bool %393
%395 = OpLogicalAnd %bool %391 %394
%396 = OpCompositeExtract %v4float %384 3
%397 = OpFOrdEqual %v4bool %396 %195
%398 = OpAll %bool %397
%399 = OpLogicalAnd %bool %395 %398
OpBranch %383
%383 = OpLabel
%400 = OpPhi %bool %false %367 %399 %382
OpStore %_0_ok %400
OpStore %_8_m11 %220
%402 = OpLoad %mat4v4float %_8_m11
%403 = OpLoad %mat4v4float %_7_m10
%404 = OpCompositeExtract %v4float %402 0
%405 = OpCompositeExtract %v4float %403 0
%406 = OpFSub %v4float %404 %405
%407 = OpCompositeExtract %v4float %402 1
%408 = OpCompositeExtract %v4float %403 1
%409 = OpFSub %v4float %407 %408
%410 = OpCompositeExtract %v4float %402 2
%411 = OpCompositeExtract %v4float %403 2
%412 = OpFSub %v4float %410 %411
%413 = OpCompositeExtract %v4float %402 3
%414 = OpCompositeExtract %v4float %403 3
%415 = OpFSub %v4float %413 %414
%416 = OpCompositeConstruct %mat4v4float %406 %409 %412 %415
OpStore %_8_m11 %416
OpSelectionMerge %418 None
OpBranchConditional %400 %417 %418
%417 = OpLabel
%419 = OpFOrdEqual %v4bool %406 %238
%420 = OpAll %bool %419
%421 = OpFOrdEqual %v4bool %409 %239
%422 = OpAll %bool %421
%423 = OpLogicalAnd %bool %420 %422
%424 = OpFOrdEqual %v4bool %412 %240
%425 = OpAll %bool %424
%426 = OpLogicalAnd %bool %423 %425
%427 = OpFOrdEqual %v4bool %415 %241
%428 = OpAll %bool %427
%429 = OpLogicalAnd %bool %426 %428
OpBranch %418
%418 = OpLabel
%430 = OpPhi %bool %false %383 %429 %417
OpStore %_0_ok %430
OpSelectionMerge %432 None
OpBranchConditional %430 %431 %432
%431 = OpLabel
%433 = OpFunctionCall %bool %test_half_b
OpBranch %432
%432 = OpLabel
%434 = OpPhi %bool %false %418 %433 %431
OpSelectionMerge %436 None
OpBranchConditional %434 %435 %436
%435 = OpLabel
%437 = OpFunctionCall %bool %test_comma_b
OpBranch %436
%436 = OpLabel
%438 = OpPhi %bool %false %432 %437 %435
OpSelectionMerge %443 None
OpBranchConditional %438 %441 %442
%441 = OpLabel
%444 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%447 = OpLoad %v4float %444
OpStore %439 %447
OpBranch %443
%442 = OpLabel
%448 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%449 = OpLoad %v4float %448
OpStore %439 %449
OpBranch %443
%443 = OpLabel
%450 = OpLoad %v4float %439
OpReturnValue %450
OpFunctionEnd
