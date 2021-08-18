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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %m1 "m1"
OpName %m2 "m2"
OpName %m3 "m3"
OpName %S "S"
OpMemberName %S 0 "x"
OpMemberName %S 1 "y"
OpName %s1 "s1"
OpName %s2 "s2"
OpName %s3 "s3"
OpDecorate %sk_FragCoord BuiltIn FragCoord
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
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %_arr_v3int_int_2 ArrayStride 16
OpDecorate %_arr_mat2v2float_int_3 ArrayStride 32
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %268 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %302 RelaxedPrecision
OpDecorate %303 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %343 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_n4 = OpConstant %float -4
%v3int = OpTypeVector %int 3
%int_2 = OpConstant %int 2
%_arr_v3int_int_2 = OpTypeArray %v3int %int_2
%_ptr_Function__arr_v3int_int_2 = OpTypePointer Function %_arr_v3int_int_2
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%50 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%53 = OpConstantComposite %v3int %int_4 %int_5 %int_6
%int_n6 = OpConstant %int -6
%59 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3
%_ptr_Function__arr_mat2v2float_int_3 = OpTypePointer Function %_arr_mat2v2float_int_3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%f1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_4 Function
%f3 = OpVariable %_ptr_Function__arr_float_int_4 Function
%v1 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%v2 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%v3 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
%m1 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%m2 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%m3 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
%s1 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s2 = OpVariable %_ptr_Function__arr_S_int_3 Function
%s3 = OpVariable %_ptr_Function__arr_S_int_3 Function
%334 = OpVariable %_ptr_Function_v4float Function
%37 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %37
%39 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f2 %39
%42 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %42
%54 = OpCompositeConstruct %_arr_v3int_int_2 %50 %53
OpStore %v1 %54
%56 = OpCompositeConstruct %_arr_v3int_int_2 %50 %53
OpStore %v2 %56
%60 = OpCompositeConstruct %_arr_v3int_int_2 %50 %59
OpStore %v3 %60
%66 = OpCompositeConstruct %v2float %float_1 %float_0
%67 = OpCompositeConstruct %v2float %float_0 %float_1
%65 = OpCompositeConstruct %mat2v2float %66 %67
%69 = OpCompositeConstruct %v2float %float_2 %float_0
%70 = OpCompositeConstruct %v2float %float_0 %float_2
%68 = OpCompositeConstruct %mat2v2float %69 %70
%73 = OpCompositeConstruct %v2float %float_3 %float_4
%74 = OpCompositeConstruct %v2float %float_5 %float_6
%75 = OpCompositeConstruct %mat2v2float %73 %74
%76 = OpCompositeConstruct %_arr_mat2v2float_int_3 %65 %68 %75
OpStore %m1 %76
%79 = OpCompositeConstruct %v2float %float_1 %float_0
%80 = OpCompositeConstruct %v2float %float_0 %float_1
%78 = OpCompositeConstruct %mat2v2float %79 %80
%82 = OpCompositeConstruct %v2float %float_2 %float_0
%83 = OpCompositeConstruct %v2float %float_0 %float_2
%81 = OpCompositeConstruct %mat2v2float %82 %83
%84 = OpCompositeConstruct %v2float %float_3 %float_4
%85 = OpCompositeConstruct %v2float %float_5 %float_6
%86 = OpCompositeConstruct %mat2v2float %84 %85
%87 = OpCompositeConstruct %_arr_mat2v2float_int_3 %78 %81 %86
OpStore %m2 %87
%90 = OpCompositeConstruct %v2float %float_1 %float_0
%91 = OpCompositeConstruct %v2float %float_0 %float_1
%89 = OpCompositeConstruct %mat2v2float %90 %91
%92 = OpCompositeConstruct %v2float %float_2 %float_3
%93 = OpCompositeConstruct %v2float %float_4 %float_5
%94 = OpCompositeConstruct %mat2v2float %92 %93
%96 = OpCompositeConstruct %v2float %float_6 %float_0
%97 = OpCompositeConstruct %v2float %float_0 %float_6
%95 = OpCompositeConstruct %mat2v2float %96 %97
%98 = OpCompositeConstruct %_arr_mat2v2float_int_3 %89 %94 %95
OpStore %m3 %98
%103 = OpCompositeConstruct %S %int_1 %int_2
%104 = OpCompositeConstruct %S %int_3 %int_4
%105 = OpCompositeConstruct %S %int_5 %int_6
%106 = OpCompositeConstruct %_arr_S_int_3 %103 %104 %105
OpStore %s1 %106
%108 = OpCompositeConstruct %S %int_1 %int_2
%110 = OpCompositeConstruct %S %int_0 %int_0
%111 = OpCompositeConstruct %S %int_5 %int_6
%112 = OpCompositeConstruct %_arr_S_int_3 %108 %110 %111
OpStore %s2 %112
%114 = OpCompositeConstruct %S %int_1 %int_2
%115 = OpCompositeConstruct %S %int_3 %int_4
%116 = OpCompositeConstruct %S %int_5 %int_6
%117 = OpCompositeConstruct %_arr_S_int_3 %114 %115 %116
OpStore %s3 %117
%119 = OpLoad %_arr_float_int_4 %f1
%120 = OpLoad %_arr_float_int_4 %f2
%121 = OpCompositeExtract %float %119 0
%122 = OpCompositeExtract %float %120 0
%123 = OpFOrdEqual %bool %121 %122
%124 = OpCompositeExtract %float %119 1
%125 = OpCompositeExtract %float %120 1
%126 = OpFOrdEqual %bool %124 %125
%127 = OpLogicalAnd %bool %126 %123
%128 = OpCompositeExtract %float %119 2
%129 = OpCompositeExtract %float %120 2
%130 = OpFOrdEqual %bool %128 %129
%131 = OpLogicalAnd %bool %130 %127
%132 = OpCompositeExtract %float %119 3
%133 = OpCompositeExtract %float %120 3
%134 = OpFOrdEqual %bool %132 %133
%135 = OpLogicalAnd %bool %134 %131
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpLoad %_arr_float_int_4 %f1
%139 = OpLoad %_arr_float_int_4 %f3
%140 = OpCompositeExtract %float %138 0
%141 = OpCompositeExtract %float %139 0
%142 = OpFOrdNotEqual %bool %140 %141
%143 = OpCompositeExtract %float %138 1
%144 = OpCompositeExtract %float %139 1
%145 = OpFOrdNotEqual %bool %143 %144
%146 = OpLogicalOr %bool %145 %142
%147 = OpCompositeExtract %float %138 2
%148 = OpCompositeExtract %float %139 2
%149 = OpFOrdNotEqual %bool %147 %148
%150 = OpLogicalOr %bool %149 %146
%151 = OpCompositeExtract %float %138 3
%152 = OpCompositeExtract %float %139 3
%153 = OpFOrdNotEqual %bool %151 %152
%154 = OpLogicalOr %bool %153 %150
OpBranch %137
%137 = OpLabel
%155 = OpPhi %bool %false %27 %154 %136
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %_arr_v3int_int_2 %v1
%159 = OpLoad %_arr_v3int_int_2 %v2
%160 = OpCompositeExtract %v3int %158 0
%161 = OpCompositeExtract %v3int %159 0
%162 = OpIEqual %v3bool %160 %161
%164 = OpAll %bool %162
%165 = OpCompositeExtract %v3int %158 1
%166 = OpCompositeExtract %v3int %159 1
%167 = OpIEqual %v3bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %168 %164
OpBranch %157
%157 = OpLabel
%170 = OpPhi %bool %false %137 %169 %156
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpLoad %_arr_v3int_int_2 %v1
%174 = OpLoad %_arr_v3int_int_2 %v3
%175 = OpCompositeExtract %v3int %173 0
%176 = OpCompositeExtract %v3int %174 0
%177 = OpINotEqual %v3bool %175 %176
%178 = OpAny %bool %177
%179 = OpCompositeExtract %v3int %173 1
%180 = OpCompositeExtract %v3int %174 1
%181 = OpINotEqual %v3bool %179 %180
%182 = OpAny %bool %181
%183 = OpLogicalOr %bool %182 %178
OpBranch %172
%172 = OpLabel
%184 = OpPhi %bool %false %157 %183 %171
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpLoad %_arr_mat2v2float_int_3 %m1
%188 = OpLoad %_arr_mat2v2float_int_3 %m2
%189 = OpCompositeExtract %mat2v2float %187 0
%190 = OpCompositeExtract %mat2v2float %188 0
%192 = OpCompositeExtract %v2float %189 0
%193 = OpCompositeExtract %v2float %190 0
%194 = OpFOrdEqual %v2bool %192 %193
%195 = OpAll %bool %194
%196 = OpCompositeExtract %v2float %189 1
%197 = OpCompositeExtract %v2float %190 1
%198 = OpFOrdEqual %v2bool %196 %197
%199 = OpAll %bool %198
%200 = OpLogicalAnd %bool %195 %199
%201 = OpCompositeExtract %mat2v2float %187 1
%202 = OpCompositeExtract %mat2v2float %188 1
%203 = OpCompositeExtract %v2float %201 0
%204 = OpCompositeExtract %v2float %202 0
%205 = OpFOrdEqual %v2bool %203 %204
%206 = OpAll %bool %205
%207 = OpCompositeExtract %v2float %201 1
%208 = OpCompositeExtract %v2float %202 1
%209 = OpFOrdEqual %v2bool %207 %208
%210 = OpAll %bool %209
%211 = OpLogicalAnd %bool %206 %210
%212 = OpLogicalAnd %bool %211 %200
%213 = OpCompositeExtract %mat2v2float %187 2
%214 = OpCompositeExtract %mat2v2float %188 2
%215 = OpCompositeExtract %v2float %213 0
%216 = OpCompositeExtract %v2float %214 0
%217 = OpFOrdEqual %v2bool %215 %216
%218 = OpAll %bool %217
%219 = OpCompositeExtract %v2float %213 1
%220 = OpCompositeExtract %v2float %214 1
%221 = OpFOrdEqual %v2bool %219 %220
%222 = OpAll %bool %221
%223 = OpLogicalAnd %bool %218 %222
%224 = OpLogicalAnd %bool %223 %212
OpBranch %186
%186 = OpLabel
%225 = OpPhi %bool %false %172 %224 %185
OpSelectionMerge %227 None
OpBranchConditional %225 %226 %227
%226 = OpLabel
%228 = OpLoad %_arr_mat2v2float_int_3 %m1
%229 = OpLoad %_arr_mat2v2float_int_3 %m3
%230 = OpCompositeExtract %mat2v2float %228 0
%231 = OpCompositeExtract %mat2v2float %229 0
%232 = OpCompositeExtract %v2float %230 0
%233 = OpCompositeExtract %v2float %231 0
%234 = OpFOrdNotEqual %v2bool %232 %233
%235 = OpAny %bool %234
%236 = OpCompositeExtract %v2float %230 1
%237 = OpCompositeExtract %v2float %231 1
%238 = OpFOrdNotEqual %v2bool %236 %237
%239 = OpAny %bool %238
%240 = OpLogicalOr %bool %235 %239
%241 = OpCompositeExtract %mat2v2float %228 1
%242 = OpCompositeExtract %mat2v2float %229 1
%243 = OpCompositeExtract %v2float %241 0
%244 = OpCompositeExtract %v2float %242 0
%245 = OpFOrdNotEqual %v2bool %243 %244
%246 = OpAny %bool %245
%247 = OpCompositeExtract %v2float %241 1
%248 = OpCompositeExtract %v2float %242 1
%249 = OpFOrdNotEqual %v2bool %247 %248
%250 = OpAny %bool %249
%251 = OpLogicalOr %bool %246 %250
%252 = OpLogicalOr %bool %251 %240
%253 = OpCompositeExtract %mat2v2float %228 2
%254 = OpCompositeExtract %mat2v2float %229 2
%255 = OpCompositeExtract %v2float %253 0
%256 = OpCompositeExtract %v2float %254 0
%257 = OpFOrdNotEqual %v2bool %255 %256
%258 = OpAny %bool %257
%259 = OpCompositeExtract %v2float %253 1
%260 = OpCompositeExtract %v2float %254 1
%261 = OpFOrdNotEqual %v2bool %259 %260
%262 = OpAny %bool %261
%263 = OpLogicalOr %bool %258 %262
%264 = OpLogicalOr %bool %263 %252
OpBranch %227
%227 = OpLabel
%265 = OpPhi %bool %false %186 %264 %226
OpSelectionMerge %267 None
OpBranchConditional %265 %266 %267
%266 = OpLabel
%268 = OpLoad %_arr_S_int_3 %s1
%269 = OpLoad %_arr_S_int_3 %s2
%270 = OpCompositeExtract %S %268 0
%271 = OpCompositeExtract %S %269 0
%272 = OpCompositeExtract %int %270 0
%273 = OpCompositeExtract %int %271 0
%274 = OpINotEqual %bool %272 %273
%275 = OpCompositeExtract %int %270 1
%276 = OpCompositeExtract %int %271 1
%277 = OpINotEqual %bool %275 %276
%278 = OpLogicalOr %bool %277 %274
%279 = OpCompositeExtract %S %268 1
%280 = OpCompositeExtract %S %269 1
%281 = OpCompositeExtract %int %279 0
%282 = OpCompositeExtract %int %280 0
%283 = OpINotEqual %bool %281 %282
%284 = OpCompositeExtract %int %279 1
%285 = OpCompositeExtract %int %280 1
%286 = OpINotEqual %bool %284 %285
%287 = OpLogicalOr %bool %286 %283
%288 = OpLogicalOr %bool %287 %278
%289 = OpCompositeExtract %S %268 2
%290 = OpCompositeExtract %S %269 2
%291 = OpCompositeExtract %int %289 0
%292 = OpCompositeExtract %int %290 0
%293 = OpINotEqual %bool %291 %292
%294 = OpCompositeExtract %int %289 1
%295 = OpCompositeExtract %int %290 1
%296 = OpINotEqual %bool %294 %295
%297 = OpLogicalOr %bool %296 %293
%298 = OpLogicalOr %bool %297 %288
OpBranch %267
%267 = OpLabel
%299 = OpPhi %bool %false %227 %298 %266
OpSelectionMerge %301 None
OpBranchConditional %299 %300 %301
%300 = OpLabel
%302 = OpLoad %_arr_S_int_3 %s3
%303 = OpLoad %_arr_S_int_3 %s1
%304 = OpCompositeExtract %S %302 0
%305 = OpCompositeExtract %S %303 0
%306 = OpCompositeExtract %int %304 0
%307 = OpCompositeExtract %int %305 0
%308 = OpIEqual %bool %306 %307
%309 = OpCompositeExtract %int %304 1
%310 = OpCompositeExtract %int %305 1
%311 = OpIEqual %bool %309 %310
%312 = OpLogicalAnd %bool %311 %308
%313 = OpCompositeExtract %S %302 1
%314 = OpCompositeExtract %S %303 1
%315 = OpCompositeExtract %int %313 0
%316 = OpCompositeExtract %int %314 0
%317 = OpIEqual %bool %315 %316
%318 = OpCompositeExtract %int %313 1
%319 = OpCompositeExtract %int %314 1
%320 = OpIEqual %bool %318 %319
%321 = OpLogicalAnd %bool %320 %317
%322 = OpLogicalAnd %bool %321 %312
%323 = OpCompositeExtract %S %302 2
%324 = OpCompositeExtract %S %303 2
%325 = OpCompositeExtract %int %323 0
%326 = OpCompositeExtract %int %324 0
%327 = OpIEqual %bool %325 %326
%328 = OpCompositeExtract %int %323 1
%329 = OpCompositeExtract %int %324 1
%330 = OpIEqual %bool %328 %329
%331 = OpLogicalAnd %bool %330 %327
%332 = OpLogicalAnd %bool %331 %322
OpBranch %301
%301 = OpLabel
%333 = OpPhi %bool %false %267 %332 %300
OpSelectionMerge %338 None
OpBranchConditional %333 %336 %337
%336 = OpLabel
%339 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%341 = OpLoad %v4float %339
OpStore %334 %341
OpBranch %338
%337 = OpLabel
%342 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%343 = OpLoad %v4float %342
OpStore %334 %343
OpBranch %338
%338 = OpLabel
%344 = OpLoad %v4float %334
OpReturnValue %344
OpFunctionEnd
