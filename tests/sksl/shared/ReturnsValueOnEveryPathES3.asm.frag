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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %return_on_both_sides_b "return_on_both_sides_b"
OpName %for_inside_body_b "for_inside_body_b"
OpName %x "x"
OpName %after_for_body_b "after_for_body_b"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
OpName %x_1 "x"
OpName %if_else_chain_b "if_else_chain_b"
OpName %conditional_inside_while_loop_b "conditional_inside_while_loop_b"
OpName %inside_do_loop_b "inside_do_loop_b"
OpName %inside_while_loop_b "inside_while_loop_b"
OpName %after_do_loop_b "after_do_loop_b"
OpName %after_while_loop_b "after_while_loop_b"
OpName %switch_with_all_returns_b "switch_with_all_returns_b"
OpName %switch_only_default_b "switch_only_default_b"
OpName %switch_fallthrough_b "switch_fallthrough_b"
OpName %switch_fallthrough_twice_b "switch_fallthrough_twice_b"
OpName %switch_with_break_in_loop_b "switch_with_break_in_loop_b"
OpName %x_2 "x"
OpName %switch_with_continue_in_loop_b "switch_with_continue_in_loop_b"
OpName %x_3 "x"
OpName %switch_with_if_that_returns_b "switch_with_if_that_returns_b"
OpName %switch_with_one_sided_if_then_fallthrough_b "switch_with_one_sided_if_then_fallthrough_b"
OpName %main "main"
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
OpDecorate %_UniformBuffer Block
OpDecorate %30 Binding 0
OpDecorate %30 DescriptorSet 0
OpDecorate %49 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %336 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%30 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%35 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%39 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%43 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_123 = OpConstant %float 123
%251 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %35
%36 = OpLabel
%40 = OpVariable %_ptr_Function_v2float Function
OpStore %40 %39
%42 = OpFunctionCall %v4float %main %40
OpStore %sk_FragColor %42
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %43
%44 = OpLabel
%45 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%49 = OpLoad %float %45
%51 = OpFOrdEqual %bool %49 %float_1
OpSelectionMerge %54 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
OpReturnValue %true
%53 = OpLabel
OpReturnValue %true
%54 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %43
%56 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %60
%60 = OpLabel
OpLoopMerge %64 %63 None
OpBranch %61
%61 = OpLabel
%65 = OpLoad %int %x
%67 = OpSLessThanEqual %bool %65 %int_10
OpBranchConditional %67 %62 %64
%62 = OpLabel
OpReturnValue %true
%63 = OpLabel
%69 = OpLoad %int %x
%70 = OpIAdd %int %69 %int_1
OpStore %x %70
OpBranch %60
%64 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %43
%71 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %73
%73 = OpLabel
OpLoopMerge %77 %76 None
OpBranch %74
%74 = OpLabel
%78 = OpLoad %int %x_0
%79 = OpSLessThanEqual %bool %78 %int_10
OpBranchConditional %79 %75 %77
%75 = OpLabel
OpBranch %76
%76 = OpLabel
%80 = OpLoad %int %x_0
%81 = OpIAdd %int %80 %int_1
OpStore %x_0 %81
OpBranch %73
%77 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %43
%82 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %84
%84 = OpLabel
OpLoopMerge %88 %87 None
OpBranch %85
%85 = OpLabel
%89 = OpLoad %int %x_1
%90 = OpSLessThanEqual %bool %89 %int_10
OpBranchConditional %90 %86 %88
%86 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%92 = OpLoad %float %91
%93 = OpFOrdEqual %bool %92 %float_1
OpSelectionMerge %96 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
OpReturnValue %true
%95 = OpLabel
OpReturnValue %true
%96 = OpLabel
OpBranch %87
%87 = OpLabel
%97 = OpLoad %int %x_1
%98 = OpIAdd %int %97 %int_1
OpStore %x_1 %98
OpBranch %84
%88 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %43
%99 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%101 = OpLoad %float %100
%102 = OpFOrdEqual %bool %101 %float_1
OpSelectionMerge %105 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
OpReturnValue %true
%104 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%107 = OpLoad %float %106
%109 = OpFOrdEqual %bool %107 %float_2
OpSelectionMerge %112 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
OpReturnValue %false
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%115 = OpLoad %float %114
%117 = OpFOrdEqual %bool %115 %float_3
OpSelectionMerge %120 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
OpReturnValue %true
%119 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%122 = OpLoad %float %121
%124 = OpFOrdEqual %bool %122 %float_4
OpSelectionMerge %127 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
OpReturnValue %false
%126 = OpLabel
OpReturnValue %true
%127 = OpLabel
OpBranch %120
%120 = OpLabel
OpBranch %112
%112 = OpLabel
OpBranch %105
%105 = OpLabel
OpUnreachable
OpFunctionEnd
%conditional_inside_while_loop_b = OpFunction %bool None %43
%128 = OpLabel
OpBranch %129
%129 = OpLabel
OpLoopMerge %133 %132 None
OpBranch %130
%130 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%135 = OpLoad %float %134
%137 = OpFOrdEqual %bool %135 %float_123
OpBranchConditional %137 %131 %133
%131 = OpLabel
OpReturnValue %true
%132 = OpLabel
OpBranch %129
%133 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_do_loop_b = OpFunction %bool None %43
%138 = OpLabel
OpBranch %139
%139 = OpLabel
OpLoopMerge %143 %142 None
OpBranch %140
%140 = OpLabel
OpReturnValue %true
%141 = OpLabel
OpBranch %142
%142 = OpLabel
OpBranchConditional %true %139 %143
%143 = OpLabel
OpUnreachable
OpFunctionEnd
%inside_while_loop_b = OpFunction %bool None %43
%144 = OpLabel
OpBranch %145
%145 = OpLabel
OpLoopMerge %149 %148 None
OpBranch %146
%146 = OpLabel
OpBranchConditional %true %147 %149
%147 = OpLabel
OpReturnValue %true
%148 = OpLabel
OpBranch %145
%149 = OpLabel
OpUnreachable
OpFunctionEnd
%after_do_loop_b = OpFunction %bool None %43
%150 = OpLabel
OpBranch %151
%151 = OpLabel
OpLoopMerge %155 %154 None
OpBranch %152
%152 = OpLabel
OpBranch %155
%153 = OpLabel
OpBranch %154
%154 = OpLabel
OpBranchConditional %true %151 %155
%155 = OpLabel
OpReturnValue %true
OpFunctionEnd
%after_while_loop_b = OpFunction %bool None %43
%156 = OpLabel
OpBranch %157
%157 = OpLabel
OpLoopMerge %161 %160 None
OpBranch %158
%158 = OpLabel
OpBranchConditional %true %159 %161
%159 = OpLabel
OpBranch %161
%160 = OpLabel
OpBranch %157
%161 = OpLabel
OpReturnValue %true
OpFunctionEnd
%switch_with_all_returns_b = OpFunction %bool None %43
%162 = OpLabel
%163 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%164 = OpLoad %float %163
%165 = OpConvertFToS %int %164
OpSelectionMerge %166 None
OpSwitch %165 %169 1 %167 2 %168
%167 = OpLabel
OpReturnValue %true
%168 = OpLabel
OpReturnValue %true
%169 = OpLabel
OpReturnValue %true
%166 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_only_default_b = OpFunction %bool None %43
%170 = OpLabel
%171 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%172 = OpLoad %float %171
%173 = OpConvertFToS %int %172
OpSelectionMerge %174 None
OpSwitch %173 %175
%175 = OpLabel
OpReturnValue %true
%174 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_b = OpFunction %bool None %43
%176 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%178 = OpLoad %float %177
%179 = OpConvertFToS %int %178
OpSelectionMerge %180 None
OpSwitch %179 %183 1 %181 2 %182
%181 = OpLabel
OpReturnValue %true
%182 = OpLabel
OpBranch %183
%183 = OpLabel
OpReturnValue %true
%180 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_b = OpFunction %bool None %43
%184 = OpLabel
%185 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%186 = OpLoad %float %185
%187 = OpConvertFToS %int %186
OpSelectionMerge %188 None
OpSwitch %187 %191 1 %189 2 %190
%189 = OpLabel
OpBranch %190
%190 = OpLabel
OpBranch %191
%191 = OpLabel
OpReturnValue %true
%188 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_break_in_loop_b = OpFunction %bool None %43
%192 = OpLabel
%x_2 = OpVariable %_ptr_Function_int Function
%193 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%194 = OpLoad %float %193
%195 = OpConvertFToS %int %194
OpSelectionMerge %196 None
OpSwitch %195 %198 1 %197
%197 = OpLabel
OpStore %x_2 %int_0
OpBranch %200
%200 = OpLabel
OpLoopMerge %204 %203 None
OpBranch %201
%201 = OpLabel
%205 = OpLoad %int %x_2
%206 = OpSLessThanEqual %bool %205 %int_10
OpBranchConditional %206 %202 %204
%202 = OpLabel
OpBranch %204
%203 = OpLabel
%207 = OpLoad %int %x_2
%208 = OpIAdd %int %207 %int_1
OpStore %x_2 %208
OpBranch %200
%204 = OpLabel
OpBranch %198
%198 = OpLabel
OpReturnValue %true
%196 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_continue_in_loop_b = OpFunction %bool None %43
%209 = OpLabel
%x_3 = OpVariable %_ptr_Function_int Function
%210 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%211 = OpLoad %float %210
%212 = OpConvertFToS %int %211
OpSelectionMerge %213 None
OpSwitch %212 %215 1 %214
%214 = OpLabel
OpStore %x_3 %int_0
OpBranch %217
%217 = OpLabel
OpLoopMerge %221 %220 None
OpBranch %218
%218 = OpLabel
%222 = OpLoad %int %x_3
%223 = OpSLessThanEqual %bool %222 %int_10
OpBranchConditional %223 %219 %221
%219 = OpLabel
OpBranch %220
%220 = OpLabel
%224 = OpLoad %int %x_3
%225 = OpIAdd %int %224 %int_1
OpStore %x_3 %225
OpBranch %217
%221 = OpLabel
OpBranch %215
%215 = OpLabel
OpReturnValue %true
%213 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_if_that_returns_b = OpFunction %bool None %43
%226 = OpLabel
%227 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%228 = OpLoad %float %227
%229 = OpConvertFToS %int %228
OpSelectionMerge %230 None
OpSwitch %229 %232 1 %231
%231 = OpLabel
%233 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%234 = OpLoad %float %233
%235 = OpFOrdEqual %bool %234 %float_123
OpSelectionMerge %238 None
OpBranchConditional %235 %236 %237
%236 = OpLabel
OpReturnValue %true
%237 = OpLabel
OpReturnValue %true
%238 = OpLabel
OpBranch %232
%232 = OpLabel
OpReturnValue %true
%230 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_with_one_sided_if_then_fallthrough_b = OpFunction %bool None %43
%239 = OpLabel
%240 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%241 = OpLoad %float %240
%242 = OpConvertFToS %int %241
OpSelectionMerge %243 None
OpSwitch %242 %245 1 %244
%244 = OpLabel
%246 = OpAccessChain %_ptr_Uniform_float %30 %int_2
%247 = OpLoad %float %246
%248 = OpFOrdEqual %bool %247 %float_123
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
OpReturnValue %true
%250 = OpLabel
OpBranch %245
%245 = OpLabel
OpReturnValue %true
%243 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %251
%252 = OpFunctionParameter %_ptr_Function_v2float
%253 = OpLabel
%326 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %255 None
OpBranchConditional %true %254 %255
%254 = OpLabel
%256 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %255
%255 = OpLabel
%257 = OpPhi %bool %false %253 %256 %254
OpSelectionMerge %259 None
OpBranchConditional %257 %258 %259
%258 = OpLabel
%260 = OpFunctionCall %bool %for_inside_body_b
OpBranch %259
%259 = OpLabel
%261 = OpPhi %bool %false %255 %260 %258
OpSelectionMerge %263 None
OpBranchConditional %261 %262 %263
%262 = OpLabel
%264 = OpFunctionCall %bool %after_for_body_b
OpBranch %263
%263 = OpLabel
%265 = OpPhi %bool %false %259 %264 %262
OpSelectionMerge %267 None
OpBranchConditional %265 %266 %267
%266 = OpLabel
%268 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %267
%267 = OpLabel
%269 = OpPhi %bool %false %263 %268 %266
OpSelectionMerge %271 None
OpBranchConditional %269 %270 %271
%270 = OpLabel
%272 = OpFunctionCall %bool %if_else_chain_b
OpBranch %271
%271 = OpLabel
%273 = OpPhi %bool %false %267 %272 %270
OpSelectionMerge %275 None
OpBranchConditional %273 %274 %275
%274 = OpLabel
%276 = OpFunctionCall %bool %conditional_inside_while_loop_b
OpBranch %275
%275 = OpLabel
%277 = OpPhi %bool %false %271 %276 %274
OpSelectionMerge %279 None
OpBranchConditional %277 %278 %279
%278 = OpLabel
%280 = OpFunctionCall %bool %inside_do_loop_b
OpBranch %279
%279 = OpLabel
%281 = OpPhi %bool %false %275 %280 %278
OpSelectionMerge %283 None
OpBranchConditional %281 %282 %283
%282 = OpLabel
%284 = OpFunctionCall %bool %inside_while_loop_b
OpBranch %283
%283 = OpLabel
%285 = OpPhi %bool %false %279 %284 %282
OpSelectionMerge %287 None
OpBranchConditional %285 %286 %287
%286 = OpLabel
%288 = OpFunctionCall %bool %after_do_loop_b
OpBranch %287
%287 = OpLabel
%289 = OpPhi %bool %false %283 %288 %286
OpSelectionMerge %291 None
OpBranchConditional %289 %290 %291
%290 = OpLabel
%292 = OpFunctionCall %bool %after_while_loop_b
OpBranch %291
%291 = OpLabel
%293 = OpPhi %bool %false %287 %292 %290
OpSelectionMerge %295 None
OpBranchConditional %293 %294 %295
%294 = OpLabel
%296 = OpFunctionCall %bool %switch_with_all_returns_b
OpBranch %295
%295 = OpLabel
%297 = OpPhi %bool %false %291 %296 %294
OpSelectionMerge %299 None
OpBranchConditional %297 %298 %299
%298 = OpLabel
%300 = OpFunctionCall %bool %switch_only_default_b
OpBranch %299
%299 = OpLabel
%301 = OpPhi %bool %false %295 %300 %298
OpSelectionMerge %303 None
OpBranchConditional %301 %302 %303
%302 = OpLabel
%304 = OpFunctionCall %bool %switch_fallthrough_b
OpBranch %303
%303 = OpLabel
%305 = OpPhi %bool %false %299 %304 %302
OpSelectionMerge %307 None
OpBranchConditional %305 %306 %307
%306 = OpLabel
%308 = OpFunctionCall %bool %switch_fallthrough_twice_b
OpBranch %307
%307 = OpLabel
%309 = OpPhi %bool %false %303 %308 %306
OpSelectionMerge %311 None
OpBranchConditional %309 %310 %311
%310 = OpLabel
%312 = OpFunctionCall %bool %switch_with_break_in_loop_b
OpBranch %311
%311 = OpLabel
%313 = OpPhi %bool %false %307 %312 %310
OpSelectionMerge %315 None
OpBranchConditional %313 %314 %315
%314 = OpLabel
%316 = OpFunctionCall %bool %switch_with_continue_in_loop_b
OpBranch %315
%315 = OpLabel
%317 = OpPhi %bool %false %311 %316 %314
OpSelectionMerge %319 None
OpBranchConditional %317 %318 %319
%318 = OpLabel
%320 = OpFunctionCall %bool %switch_with_if_that_returns_b
OpBranch %319
%319 = OpLabel
%321 = OpPhi %bool %false %315 %320 %318
OpSelectionMerge %323 None
OpBranchConditional %321 %322 %323
%322 = OpLabel
%324 = OpFunctionCall %bool %switch_with_one_sided_if_then_fallthrough_b
OpBranch %323
%323 = OpLabel
%325 = OpPhi %bool %false %319 %324 %322
OpSelectionMerge %330 None
OpBranchConditional %325 %328 %329
%328 = OpLabel
%331 = OpAccessChain %_ptr_Uniform_v4float %30 %int_0
%333 = OpLoad %v4float %331
OpStore %326 %333
OpBranch %330
%329 = OpLabel
%334 = OpAccessChain %_ptr_Uniform_v4float %30 %int_1
%335 = OpLoad %v4float %334
OpStore %326 %335
OpBranch %330
%330 = OpLabel
%336 = OpLoad %v4float %326
OpReturnValue %336
OpFunctionEnd
