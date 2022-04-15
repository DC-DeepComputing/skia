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
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %_arr_v3int_int_2 ArrayStride 16
OpDecorate %_arr_mat2v2float_int_3 ArrayStride 32
OpMemberDecorate %S 0 Offset 0
OpMemberDecorate %S 1 Offset 4
OpDecorate %_arr_S_int_3 ArrayStride 16
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %308 RelaxedPrecision
OpDecorate %310 RelaxedPrecision
OpDecorate %311 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
%47 = OpConstantComposite %v3int %int_1 %int_2 %int_3
%int_5 = OpConstant %int 5
%int_6 = OpConstant %int 6
%50 = OpConstantComposite %v3int %int_4 %int_5 %int_6
%int_n6 = OpConstant %int -6
%55 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3
%_ptr_Function__arr_mat2v2float_int_3 = OpTypePointer Function %_arr_mat2v2float_int_3
%61 = OpConstantComposite %v2float %float_1 %float_0
%62 = OpConstantComposite %v2float %float_0 %float_1
%63 = OpConstantComposite %mat2v2float %61 %62
%64 = OpConstantComposite %v2float %float_2 %float_0
%65 = OpConstantComposite %v2float %float_0 %float_2
%66 = OpConstantComposite %mat2v2float %64 %65
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%69 = OpConstantComposite %v2float %float_3 %float_4
%70 = OpConstantComposite %v2float %float_5 %float_6
%71 = OpConstantComposite %mat2v2float %69 %70
%75 = OpConstantComposite %v2float %float_2 %float_3
%76 = OpConstantComposite %v2float %float_4 %float_5
%77 = OpConstantComposite %mat2v2float %75 %76
%78 = OpConstantComposite %v2float %float_6 %float_0
%79 = OpConstantComposite %v2float %float_0 %float_6
%80 = OpConstantComposite %mat2v2float %78 %79
%S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
%int_0 = OpConstant %int 0
%false = OpConstantFalse %bool
%v3bool = OpTypeVector %bool 3
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%301 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %f1 %35
OpStore %f2 %35
%39 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_n4
OpStore %f3 %39
%51 = OpCompositeConstruct %_arr_v3int_int_2 %47 %50
OpStore %v1 %51
OpStore %v2 %51
%56 = OpCompositeConstruct %_arr_v3int_int_2 %47 %55
OpStore %v3 %56
%72 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %66 %71
OpStore %m1 %72
OpStore %m2 %72
%81 = OpCompositeConstruct %_arr_mat2v2float_int_3 %63 %77 %80
OpStore %m3 %81
%86 = OpCompositeConstruct %S %int_1 %int_2
%87 = OpCompositeConstruct %S %int_3 %int_4
%88 = OpCompositeConstruct %S %int_5 %int_6
%89 = OpCompositeConstruct %_arr_S_int_3 %86 %87 %88
OpStore %s1 %89
%92 = OpCompositeConstruct %S %int_0 %int_0
%93 = OpCompositeConstruct %_arr_S_int_3 %86 %92 %88
OpStore %s2 %93
OpStore %s3 %89
%96 = OpFOrdEqual %bool %float_1 %float_1
%97 = OpFOrdEqual %bool %float_2 %float_2
%98 = OpLogicalAnd %bool %97 %96
%99 = OpFOrdEqual %bool %float_3 %float_3
%100 = OpLogicalAnd %bool %99 %98
%101 = OpFOrdEqual %bool %float_4 %float_4
%102 = OpLogicalAnd %bool %101 %100
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %_arr_float_int_4 %f1
%106 = OpLoad %_arr_float_int_4 %f3
%107 = OpCompositeExtract %float %105 0
%108 = OpCompositeExtract %float %106 0
%109 = OpFUnordNotEqual %bool %107 %108
%110 = OpCompositeExtract %float %105 1
%111 = OpCompositeExtract %float %106 1
%112 = OpFUnordNotEqual %bool %110 %111
%113 = OpLogicalOr %bool %112 %109
%114 = OpCompositeExtract %float %105 2
%115 = OpCompositeExtract %float %106 2
%116 = OpFUnordNotEqual %bool %114 %115
%117 = OpLogicalOr %bool %116 %113
%118 = OpCompositeExtract %float %105 3
%119 = OpCompositeExtract %float %106 3
%120 = OpFUnordNotEqual %bool %118 %119
%121 = OpLogicalOr %bool %120 %117
OpBranch %104
%104 = OpLabel
%122 = OpPhi %bool %false %25 %121 %103
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpLoad %_arr_v3int_int_2 %v1
%126 = OpLoad %_arr_v3int_int_2 %v2
%127 = OpCompositeExtract %v3int %125 0
%128 = OpCompositeExtract %v3int %126 0
%129 = OpIEqual %v3bool %127 %128
%131 = OpAll %bool %129
%132 = OpCompositeExtract %v3int %125 1
%133 = OpCompositeExtract %v3int %126 1
%134 = OpIEqual %v3bool %132 %133
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %135 %131
OpBranch %124
%124 = OpLabel
%137 = OpPhi %bool %false %104 %136 %123
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%140 = OpLoad %_arr_v3int_int_2 %v1
%141 = OpLoad %_arr_v3int_int_2 %v3
%142 = OpCompositeExtract %v3int %140 0
%143 = OpCompositeExtract %v3int %141 0
%144 = OpINotEqual %v3bool %142 %143
%145 = OpAny %bool %144
%146 = OpCompositeExtract %v3int %140 1
%147 = OpCompositeExtract %v3int %141 1
%148 = OpINotEqual %v3bool %146 %147
%149 = OpAny %bool %148
%150 = OpLogicalOr %bool %149 %145
OpBranch %139
%139 = OpLabel
%151 = OpPhi %bool %false %124 %150 %138
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpLoad %_arr_mat2v2float_int_3 %m1
%155 = OpLoad %_arr_mat2v2float_int_3 %m2
%156 = OpCompositeExtract %mat2v2float %154 0
%157 = OpCompositeExtract %mat2v2float %155 0
%159 = OpCompositeExtract %v2float %156 0
%160 = OpCompositeExtract %v2float %157 0
%161 = OpFOrdEqual %v2bool %159 %160
%162 = OpAll %bool %161
%163 = OpCompositeExtract %v2float %156 1
%164 = OpCompositeExtract %v2float %157 1
%165 = OpFOrdEqual %v2bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %162 %166
%168 = OpCompositeExtract %mat2v2float %154 1
%169 = OpCompositeExtract %mat2v2float %155 1
%170 = OpCompositeExtract %v2float %168 0
%171 = OpCompositeExtract %v2float %169 0
%172 = OpFOrdEqual %v2bool %170 %171
%173 = OpAll %bool %172
%174 = OpCompositeExtract %v2float %168 1
%175 = OpCompositeExtract %v2float %169 1
%176 = OpFOrdEqual %v2bool %174 %175
%177 = OpAll %bool %176
%178 = OpLogicalAnd %bool %173 %177
%179 = OpLogicalAnd %bool %178 %167
%180 = OpCompositeExtract %mat2v2float %154 2
%181 = OpCompositeExtract %mat2v2float %155 2
%182 = OpCompositeExtract %v2float %180 0
%183 = OpCompositeExtract %v2float %181 0
%184 = OpFOrdEqual %v2bool %182 %183
%185 = OpAll %bool %184
%186 = OpCompositeExtract %v2float %180 1
%187 = OpCompositeExtract %v2float %181 1
%188 = OpFOrdEqual %v2bool %186 %187
%189 = OpAll %bool %188
%190 = OpLogicalAnd %bool %185 %189
%191 = OpLogicalAnd %bool %190 %179
OpBranch %153
%153 = OpLabel
%192 = OpPhi %bool %false %139 %191 %152
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpLoad %_arr_mat2v2float_int_3 %m1
%196 = OpLoad %_arr_mat2v2float_int_3 %m3
%197 = OpCompositeExtract %mat2v2float %195 0
%198 = OpCompositeExtract %mat2v2float %196 0
%199 = OpCompositeExtract %v2float %197 0
%200 = OpCompositeExtract %v2float %198 0
%201 = OpFUnordNotEqual %v2bool %199 %200
%202 = OpAny %bool %201
%203 = OpCompositeExtract %v2float %197 1
%204 = OpCompositeExtract %v2float %198 1
%205 = OpFUnordNotEqual %v2bool %203 %204
%206 = OpAny %bool %205
%207 = OpLogicalOr %bool %202 %206
%208 = OpCompositeExtract %mat2v2float %195 1
%209 = OpCompositeExtract %mat2v2float %196 1
%210 = OpCompositeExtract %v2float %208 0
%211 = OpCompositeExtract %v2float %209 0
%212 = OpFUnordNotEqual %v2bool %210 %211
%213 = OpAny %bool %212
%214 = OpCompositeExtract %v2float %208 1
%215 = OpCompositeExtract %v2float %209 1
%216 = OpFUnordNotEqual %v2bool %214 %215
%217 = OpAny %bool %216
%218 = OpLogicalOr %bool %213 %217
%219 = OpLogicalOr %bool %218 %207
%220 = OpCompositeExtract %mat2v2float %195 2
%221 = OpCompositeExtract %mat2v2float %196 2
%222 = OpCompositeExtract %v2float %220 0
%223 = OpCompositeExtract %v2float %221 0
%224 = OpFUnordNotEqual %v2bool %222 %223
%225 = OpAny %bool %224
%226 = OpCompositeExtract %v2float %220 1
%227 = OpCompositeExtract %v2float %221 1
%228 = OpFUnordNotEqual %v2bool %226 %227
%229 = OpAny %bool %228
%230 = OpLogicalOr %bool %225 %229
%231 = OpLogicalOr %bool %230 %219
OpBranch %194
%194 = OpLabel
%232 = OpPhi %bool %false %153 %231 %193
OpSelectionMerge %234 None
OpBranchConditional %232 %233 %234
%233 = OpLabel
%235 = OpLoad %_arr_S_int_3 %s1
%236 = OpLoad %_arr_S_int_3 %s2
%237 = OpCompositeExtract %S %235 0
%238 = OpCompositeExtract %S %236 0
%239 = OpCompositeExtract %int %237 0
%240 = OpCompositeExtract %int %238 0
%241 = OpINotEqual %bool %239 %240
%242 = OpCompositeExtract %int %237 1
%243 = OpCompositeExtract %int %238 1
%244 = OpINotEqual %bool %242 %243
%245 = OpLogicalOr %bool %244 %241
%246 = OpCompositeExtract %S %235 1
%247 = OpCompositeExtract %S %236 1
%248 = OpCompositeExtract %int %246 0
%249 = OpCompositeExtract %int %247 0
%250 = OpINotEqual %bool %248 %249
%251 = OpCompositeExtract %int %246 1
%252 = OpCompositeExtract %int %247 1
%253 = OpINotEqual %bool %251 %252
%254 = OpLogicalOr %bool %253 %250
%255 = OpLogicalOr %bool %254 %245
%256 = OpCompositeExtract %S %235 2
%257 = OpCompositeExtract %S %236 2
%258 = OpCompositeExtract %int %256 0
%259 = OpCompositeExtract %int %257 0
%260 = OpINotEqual %bool %258 %259
%261 = OpCompositeExtract %int %256 1
%262 = OpCompositeExtract %int %257 1
%263 = OpINotEqual %bool %261 %262
%264 = OpLogicalOr %bool %263 %260
%265 = OpLogicalOr %bool %264 %255
OpBranch %234
%234 = OpLabel
%266 = OpPhi %bool %false %194 %265 %233
OpSelectionMerge %268 None
OpBranchConditional %266 %267 %268
%267 = OpLabel
%269 = OpLoad %_arr_S_int_3 %s3
%270 = OpLoad %_arr_S_int_3 %s1
%271 = OpCompositeExtract %S %269 0
%272 = OpCompositeExtract %S %270 0
%273 = OpCompositeExtract %int %271 0
%274 = OpCompositeExtract %int %272 0
%275 = OpIEqual %bool %273 %274
%276 = OpCompositeExtract %int %271 1
%277 = OpCompositeExtract %int %272 1
%278 = OpIEqual %bool %276 %277
%279 = OpLogicalAnd %bool %278 %275
%280 = OpCompositeExtract %S %269 1
%281 = OpCompositeExtract %S %270 1
%282 = OpCompositeExtract %int %280 0
%283 = OpCompositeExtract %int %281 0
%284 = OpIEqual %bool %282 %283
%285 = OpCompositeExtract %int %280 1
%286 = OpCompositeExtract %int %281 1
%287 = OpIEqual %bool %285 %286
%288 = OpLogicalAnd %bool %287 %284
%289 = OpLogicalAnd %bool %288 %279
%290 = OpCompositeExtract %S %269 2
%291 = OpCompositeExtract %S %270 2
%292 = OpCompositeExtract %int %290 0
%293 = OpCompositeExtract %int %291 0
%294 = OpIEqual %bool %292 %293
%295 = OpCompositeExtract %int %290 1
%296 = OpCompositeExtract %int %291 1
%297 = OpIEqual %bool %295 %296
%298 = OpLogicalAnd %bool %297 %294
%299 = OpLogicalAnd %bool %298 %289
OpBranch %268
%268 = OpLabel
%300 = OpPhi %bool %false %234 %299 %267
OpSelectionMerge %305 None
OpBranchConditional %300 %303 %304
%303 = OpLabel
%306 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%308 = OpLoad %v4float %306
OpStore %301 %308
OpBranch %305
%304 = OpLabel
%309 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%310 = OpLoad %v4float %309
OpStore %301 %310
OpBranch %305
%305 = OpLabel
%311 = OpLoad %v4float %301
OpReturnValue %311
OpFunctionEnd
