OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_bifffff22 "test_bifffff22"
OpName %one "one"
OpName %m2 "m2"
OpName %divisionTest_b "divisionTest_b"
OpName %ten "ten"
OpName %div "div"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %_0_expected "_0_expected"
OpName %_1_one "_1_one"
OpName %_2_m2 "_2_m2"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %295 RelaxedPrecision
OpDecorate %297 RelaxedPrecision
OpDecorate %298 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%30 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%63 = OpConstantComposite %v2float %float_1 %float_1
%64 = OpConstantComposite %mat2v2float %63 %63
%float_2 = OpConstant %float 2
%float_0_5 = OpConstant %float 0.5
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%119 = OpTypeFunction %bool
%float_10 = OpConstant %float 10
%int_2 = OpConstant %int 2
%float_n8 = OpConstant %float -8
%140 = OpConstantComposite %v2float %float_n8 %float_n8
%141 = OpConstantComposite %mat2v2float %140 %140
%v2bool = OpTypeVector %bool 2
%148 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %30
%31 = OpFunctionParameter %_ptr_Function_int
%32 = OpFunctionParameter %_ptr_Function_float
%33 = OpFunctionParameter %_ptr_Function_float
%34 = OpFunctionParameter %_ptr_Function_float
%35 = OpFunctionParameter %_ptr_Function_float
%36 = OpFunctionParameter %_ptr_Function_mat2v2float
%37 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%39 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%42 = OpLoad %v4float %39
%43 = OpCompositeExtract %float %42 0
OpStore %one %43
%45 = OpLoad %float %32
%46 = OpFMul %float %45 %43
%47 = OpLoad %float %33
%48 = OpFMul %float %47 %43
%49 = OpLoad %float %34
%50 = OpFMul %float %49 %43
%51 = OpLoad %float %35
%52 = OpFMul %float %51 %43
%53 = OpCompositeConstruct %v2float %46 %48
%54 = OpCompositeConstruct %v2float %50 %52
%55 = OpCompositeConstruct %mat2v2float %53 %54
OpStore %m2 %55
%56 = OpLoad %int %31
OpSelectionMerge %57 None
OpSwitch %56 %57 1 %58 2 %59 3 %60 4 %61
%58 = OpLabel
%65 = OpFAdd %v2float %53 %63
%66 = OpFAdd %v2float %54 %63
%67 = OpCompositeConstruct %mat2v2float %65 %66
OpStore %m2 %67
OpBranch %57
%59 = OpLabel
%68 = OpLoad %mat2v2float %m2
%69 = OpCompositeExtract %v2float %68 0
%70 = OpFSub %v2float %69 %63
%71 = OpCompositeExtract %v2float %68 1
%72 = OpFSub %v2float %71 %63
%73 = OpCompositeConstruct %mat2v2float %70 %72
OpStore %m2 %73
OpBranch %57
%60 = OpLabel
%74 = OpLoad %mat2v2float %m2
%76 = OpMatrixTimesScalar %mat2v2float %74 %float_2
OpStore %m2 %76
OpBranch %57
%61 = OpLabel
%77 = OpLoad %mat2v2float %m2
%79 = OpMatrixTimesScalar %mat2v2float %77 %float_0_5
OpStore %m2 %79
OpBranch %57
%57 = OpLabel
%82 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%83 = OpLoad %v2float %82
%84 = OpCompositeExtract %float %83 0
%85 = OpAccessChain %_ptr_Function_v2float %36 %int_0
%86 = OpLoad %v2float %85
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdEqual %bool %84 %87
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%92 = OpLoad %v2float %91
%93 = OpCompositeExtract %float %92 1
%94 = OpAccessChain %_ptr_Function_v2float %36 %int_0
%95 = OpLoad %v2float %94
%96 = OpCompositeExtract %float %95 1
%97 = OpFOrdEqual %bool %93 %96
OpBranch %90
%90 = OpLabel
%98 = OpPhi %bool %false %57 %97 %89
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%102 = OpLoad %v2float %101
%103 = OpCompositeExtract %float %102 0
%104 = OpAccessChain %_ptr_Function_v2float %36 %int_1
%105 = OpLoad %v2float %104
%106 = OpCompositeExtract %float %105 0
%107 = OpFOrdEqual %bool %103 %106
OpBranch %100
%100 = OpLabel
%108 = OpPhi %bool %false %90 %107 %99
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%112 = OpLoad %v2float %111
%113 = OpCompositeExtract %float %112 1
%114 = OpAccessChain %_ptr_Function_v2float %36 %int_1
%115 = OpLoad %v2float %114
%116 = OpCompositeExtract %float %115 1
%117 = OpFOrdEqual %bool %113 %116
OpBranch %110
%110 = OpLabel
%118 = OpPhi %bool %false %100 %117 %109
OpReturnValue %118
OpFunctionEnd
%divisionTest_b = OpFunction %bool None %119
%120 = OpLabel
%ten = OpVariable %_ptr_Function_float Function
%div = OpVariable %_ptr_Function_mat2v2float Function
%122 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%123 = OpLoad %v4float %122
%124 = OpCompositeExtract %float %123 0
%126 = OpFMul %float %124 %float_10
OpStore %ten %126
%128 = OpCompositeConstruct %v2float %126 %126
%129 = OpCompositeConstruct %mat2v2float %128 %128
%130 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%132 = OpLoad %v4float %130
%133 = OpCompositeExtract %float %132 0
%134 = OpCompositeConstruct %v2float %133 %133
%135 = OpCompositeConstruct %mat2v2float %134 %134
%136 = OpFDiv %v2float %128 %134
%137 = OpFDiv %v2float %128 %134
%138 = OpCompositeConstruct %mat2v2float %136 %137
OpStore %div %138
%143 = OpFOrdEqual %v2bool %136 %140
%144 = OpAll %bool %143
%145 = OpFOrdEqual %v2bool %137 %140
%146 = OpAll %bool %145
%147 = OpLogicalAnd %bool %144 %146
OpReturnValue %147
OpFunctionEnd
%main = OpFunction %v4float None %148
%149 = OpFunctionParameter %_ptr_Function_v2float
%150 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%_0_expected = OpVariable %_ptr_Function_mat2v2float Function
%_1_one = OpVariable %_ptr_Function_float Function
%_2_m2 = OpVariable %_ptr_Function_mat2v2float Function
%234 = OpVariable %_ptr_Function_int Function
%235 = OpVariable %_ptr_Function_float Function
%236 = OpVariable %_ptr_Function_float Function
%237 = OpVariable %_ptr_Function_float Function
%238 = OpVariable %_ptr_Function_float Function
%246 = OpVariable %_ptr_Function_mat2v2float Function
%252 = OpVariable %_ptr_Function_int Function
%253 = OpVariable %_ptr_Function_float Function
%254 = OpVariable %_ptr_Function_float Function
%255 = OpVariable %_ptr_Function_float Function
%256 = OpVariable %_ptr_Function_float Function
%264 = OpVariable %_ptr_Function_mat2v2float Function
%270 = OpVariable %_ptr_Function_int Function
%271 = OpVariable %_ptr_Function_float Function
%272 = OpVariable %_ptr_Function_float Function
%273 = OpVariable %_ptr_Function_float Function
%274 = OpVariable %_ptr_Function_float Function
%282 = OpVariable %_ptr_Function_mat2v2float Function
%289 = OpVariable %_ptr_Function_v4float Function
%152 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%153 = OpLoad %v4float %152
%154 = OpCompositeExtract %float %153 1
OpStore %f1 %154
%156 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%157 = OpLoad %v4float %156
%158 = OpCompositeExtract %float %157 1
%159 = OpFMul %float %float_2 %158
OpStore %f2 %159
%162 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%163 = OpLoad %v4float %162
%164 = OpCompositeExtract %float %163 1
%165 = OpFMul %float %float_3 %164
OpStore %f3 %165
%168 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%169 = OpLoad %v4float %168
%170 = OpCompositeExtract %float %169 1
%171 = OpFMul %float %float_4 %170
OpStore %f4 %171
%173 = OpFAdd %float %154 %float_1
%174 = OpFAdd %float %159 %float_1
%175 = OpFAdd %float %165 %float_1
%176 = OpFAdd %float %171 %float_1
%177 = OpCompositeConstruct %v2float %173 %174
%178 = OpCompositeConstruct %v2float %175 %176
%179 = OpCompositeConstruct %mat2v2float %177 %178
OpStore %_0_expected %179
%181 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%182 = OpLoad %v4float %181
%183 = OpCompositeExtract %float %182 0
OpStore %_1_one %183
%185 = OpFMul %float %154 %183
%186 = OpFMul %float %159 %183
%187 = OpFMul %float %165 %183
%188 = OpFMul %float %171 %183
%189 = OpCompositeConstruct %v2float %185 %186
%190 = OpCompositeConstruct %v2float %187 %188
%191 = OpCompositeConstruct %mat2v2float %189 %190
OpStore %_2_m2 %191
%192 = OpFAdd %v2float %189 %63
%193 = OpFAdd %v2float %190 %63
%194 = OpCompositeConstruct %mat2v2float %192 %193
OpStore %_2_m2 %194
%195 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%196 = OpLoad %v2float %195
%197 = OpCompositeExtract %float %196 0
%198 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%199 = OpLoad %v2float %198
%200 = OpCompositeExtract %float %199 0
%201 = OpFOrdEqual %bool %197 %200
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%204 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_0
%205 = OpLoad %v2float %204
%206 = OpCompositeExtract %float %205 1
%207 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_0
%208 = OpLoad %v2float %207
%209 = OpCompositeExtract %float %208 1
%210 = OpFOrdEqual %bool %206 %209
OpBranch %203
%203 = OpLabel
%211 = OpPhi %bool %false %150 %210 %202
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%214 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%215 = OpLoad %v2float %214
%216 = OpCompositeExtract %float %215 0
%217 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%218 = OpLoad %v2float %217
%219 = OpCompositeExtract %float %218 0
%220 = OpFOrdEqual %bool %216 %219
OpBranch %213
%213 = OpLabel
%221 = OpPhi %bool %false %203 %220 %212
OpSelectionMerge %223 None
OpBranchConditional %221 %222 %223
%222 = OpLabel
%224 = OpAccessChain %_ptr_Function_v2float %_2_m2 %int_1
%225 = OpLoad %v2float %224
%226 = OpCompositeExtract %float %225 1
%227 = OpAccessChain %_ptr_Function_v2float %_0_expected %int_1
%228 = OpLoad %v2float %227
%229 = OpCompositeExtract %float %228 1
%230 = OpFOrdEqual %bool %226 %229
OpBranch %223
%223 = OpLabel
%231 = OpPhi %bool %false %213 %230 %222
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
OpStore %234 %int_2
OpStore %235 %154
OpStore %236 %159
OpStore %237 %165
OpStore %238 %171
%239 = OpFSub %float %154 %float_1
%240 = OpFSub %float %159 %float_1
%241 = OpFSub %float %165 %float_1
%242 = OpFSub %float %171 %float_1
%243 = OpCompositeConstruct %v2float %239 %240
%244 = OpCompositeConstruct %v2float %241 %242
%245 = OpCompositeConstruct %mat2v2float %243 %244
OpStore %246 %245
%247 = OpFunctionCall %bool %test_bifffff22 %234 %235 %236 %237 %238 %246
OpBranch %233
%233 = OpLabel
%248 = OpPhi %bool %false %223 %247 %232
OpSelectionMerge %250 None
OpBranchConditional %248 %249 %250
%249 = OpLabel
OpStore %252 %int_3
OpStore %253 %154
OpStore %254 %159
OpStore %255 %165
OpStore %256 %171
%257 = OpFMul %float %154 %float_2
%258 = OpFMul %float %159 %float_2
%259 = OpFMul %float %165 %float_2
%260 = OpFMul %float %171 %float_2
%261 = OpCompositeConstruct %v2float %257 %258
%262 = OpCompositeConstruct %v2float %259 %260
%263 = OpCompositeConstruct %mat2v2float %261 %262
OpStore %264 %263
%265 = OpFunctionCall %bool %test_bifffff22 %252 %253 %254 %255 %256 %264
OpBranch %250
%250 = OpLabel
%266 = OpPhi %bool %false %233 %265 %249
OpSelectionMerge %268 None
OpBranchConditional %266 %267 %268
%267 = OpLabel
OpStore %270 %int_4
OpStore %271 %154
OpStore %272 %159
OpStore %273 %165
OpStore %274 %171
%275 = OpFMul %float %154 %float_0_5
%276 = OpFMul %float %159 %float_0_5
%277 = OpFMul %float %165 %float_0_5
%278 = OpFMul %float %171 %float_0_5
%279 = OpCompositeConstruct %v2float %275 %276
%280 = OpCompositeConstruct %v2float %277 %278
%281 = OpCompositeConstruct %mat2v2float %279 %280
OpStore %282 %281
%283 = OpFunctionCall %bool %test_bifffff22 %270 %271 %272 %273 %274 %282
OpBranch %268
%268 = OpLabel
%284 = OpPhi %bool %false %250 %283 %267
OpSelectionMerge %286 None
OpBranchConditional %284 %285 %286
%285 = OpLabel
%287 = OpFunctionCall %bool %divisionTest_b
OpBranch %286
%286 = OpLabel
%288 = OpPhi %bool %false %268 %287 %285
OpSelectionMerge %293 None
OpBranchConditional %288 %291 %292
%291 = OpLabel
%294 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%295 = OpLoad %v4float %294
OpStore %289 %295
OpBranch %293
%292 = OpLabel
%296 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%297 = OpLoad %v4float %296
OpStore %289 %297
OpBranch %293
%293 = OpLabel
%298 = OpLoad %v4float %289
OpReturnValue %298
OpFunctionEnd
