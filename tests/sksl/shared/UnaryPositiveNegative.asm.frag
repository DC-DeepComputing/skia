OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpMemberName %_UniformBuffer 3 "testMatrix2x2"
OpMemberName %_UniformBuffer 4 "testMatrix3x3"
OpMemberName %_UniformBuffer 5 "testMatrix4x4"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_iscalar_b "test_iscalar_b"
OpName %x "x"
OpName %test_fvec_b "test_fvec_b"
OpName %x_0 "x"
OpName %test_ivec_b "test_ivec_b"
OpName %x_1 "x"
OpName %test_mat2_b "test_mat2_b"
OpName %negated "negated"
OpName %x_2 "x"
OpName %test_mat3_b "test_mat3_b"
OpName %negated_0 "negated"
OpName %x_3 "x"
OpName %test_mat4_b "test_mat4_b"
OpName %negated_1 "negated"
OpName %x_4 "x"
OpName %main "main"
OpName %_0_x "_0_x"
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
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 4 Offset 80
OpMemberDecorate %_UniformBuffer 4 ColMajor
OpMemberDecorate %_UniformBuffer 4 MatrixStride 16
OpMemberDecorate %_UniformBuffer 5 Offset 128
OpMemberDecorate %_UniformBuffer 5 ColMajor
OpMemberDecorate %_UniformBuffer 5 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %x_0 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %mat2v2float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%26 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%float_n1 = OpConstant %float -1
%56 = OpConstantComposite %v2float %float_n1 %float_n1
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%71 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%80 = OpConstantComposite %v2float %float_n1 %float_n2
%81 = OpConstantComposite %v2float %float_n3 %float_n4
%82 = OpConstantComposite %mat2v2float %80 %81
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n5 = OpConstant %float -5
%float_n6 = OpConstant %float -6
%float_n7 = OpConstant %float -7
%float_n8 = OpConstant %float -8
%float_n9 = OpConstant %float -9
%110 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n3
%111 = OpConstantComposite %v3float %float_n4 %float_n5 %float_n6
%112 = OpConstantComposite %v3float %float_n7 %float_n8 %float_n9
%113 = OpConstantComposite %mat3v3float %110 %111 %112
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_4 = OpConstant %int 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_n10 = OpConstant %float -10
%float_n11 = OpConstant %float -11
%float_n12 = OpConstant %float -12
%float_n13 = OpConstant %float -13
%float_n14 = OpConstant %float -14
%float_n15 = OpConstant %float -15
%float_n16 = OpConstant %float -16
%150 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%151 = OpConstantComposite %v4float %float_n5 %float_n6 %float_n7 %float_n8
%152 = OpConstantComposite %v4float %float_n9 %float_n10 %float_n11 %float_n12
%153 = OpConstantComposite %v4float %float_n13 %float_n14 %float_n15 %float_n16
%154 = OpConstantComposite %mat4v4float %150 %151 %152 %153
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_5 = OpConstant %int 5
%v4bool = OpTypeVector %bool 4
%187 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %26
%27 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%test_iscalar_b = OpFunction %bool None %33
%34 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%38 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%41 = OpLoad %v4float %38
%42 = OpCompositeExtract %float %41 0
%43 = OpConvertFToS %int %42
OpStore %x %43
%44 = OpLoad %int %x
%45 = OpSNegate %int %44
OpStore %x %45
%47 = OpIEqual %bool %45 %int_n1
OpReturnValue %47
OpFunctionEnd
%test_fvec_b = OpFunction %bool None %33
%48 = OpLabel
%x_0 = OpVariable %_ptr_Function_v2float Function
%50 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
OpStore %x_0 %52
%53 = OpLoad %v2float %x_0
%54 = OpFNegate %v2float %53
OpStore %x_0 %54
%57 = OpFOrdEqual %v2bool %54 %56
%59 = OpAll %bool %57
OpReturnValue %59
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %33
%60 = OpLabel
%x_1 = OpVariable %_ptr_Function_v2int Function
%64 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%65 = OpLoad %v4float %64
%66 = OpCompositeExtract %float %65 0
%67 = OpConvertFToS %int %66
%68 = OpCompositeConstruct %v2int %67 %67
OpStore %x_1 %68
%69 = OpLoad %v2int %x_1
%70 = OpSNegate %v2int %69
OpStore %x_1 %70
%72 = OpIEqual %v2bool %70 %71
%73 = OpAll %bool %72
OpReturnValue %73
OpFunctionEnd
%test_mat2_b = OpFunction %bool None %33
%74 = OpLabel
%negated = OpVariable %_ptr_Function_mat2v2float Function
%x_2 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %negated %82
%84 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
%87 = OpLoad %mat2v2float %84
OpStore %x_2 %87
%88 = OpLoad %mat2v2float %x_2
%89 = OpCompositeExtract %v2float %88 0
%90 = OpFNegate %v2float %89
%91 = OpCompositeExtract %v2float %88 1
%92 = OpFNegate %v2float %91
%93 = OpCompositeConstruct %mat2v2float %90 %92
OpStore %x_2 %93
%94 = OpLoad %mat2v2float %negated
%95 = OpCompositeExtract %v2float %94 0
%96 = OpFOrdEqual %v2bool %90 %95
%97 = OpAll %bool %96
%98 = OpCompositeExtract %v2float %94 1
%99 = OpFOrdEqual %v2bool %92 %98
%100 = OpAll %bool %99
%101 = OpLogicalAnd %bool %97 %100
OpReturnValue %101
OpFunctionEnd
%test_mat3_b = OpFunction %bool None %33
%102 = OpLabel
%negated_0 = OpVariable %_ptr_Function_mat3v3float Function
%x_3 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %negated_0 %113
%115 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
%118 = OpLoad %mat3v3float %115
OpStore %x_3 %118
%119 = OpLoad %mat3v3float %x_3
%120 = OpCompositeExtract %v3float %119 0
%121 = OpFNegate %v3float %120
%122 = OpCompositeExtract %v3float %119 1
%123 = OpFNegate %v3float %122
%124 = OpCompositeExtract %v3float %119 2
%125 = OpFNegate %v3float %124
%126 = OpCompositeConstruct %mat3v3float %121 %123 %125
OpStore %x_3 %126
%127 = OpLoad %mat3v3float %negated_0
%129 = OpCompositeExtract %v3float %127 0
%130 = OpFOrdEqual %v3bool %121 %129
%131 = OpAll %bool %130
%132 = OpCompositeExtract %v3float %127 1
%133 = OpFOrdEqual %v3bool %123 %132
%134 = OpAll %bool %133
%135 = OpLogicalAnd %bool %131 %134
%136 = OpCompositeExtract %v3float %127 2
%137 = OpFOrdEqual %v3bool %125 %136
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %135 %138
OpReturnValue %139
OpFunctionEnd
%test_mat4_b = OpFunction %bool None %33
%140 = OpLabel
%negated_1 = OpVariable %_ptr_Function_mat4v4float Function
%x_4 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %negated_1 %154
%156 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
%159 = OpLoad %mat4v4float %156
OpStore %x_4 %159
%160 = OpLoad %mat4v4float %x_4
%161 = OpCompositeExtract %v4float %160 0
%162 = OpFNegate %v4float %161
%163 = OpCompositeExtract %v4float %160 1
%164 = OpFNegate %v4float %163
%165 = OpCompositeExtract %v4float %160 2
%166 = OpFNegate %v4float %165
%167 = OpCompositeExtract %v4float %160 3
%168 = OpFNegate %v4float %167
%169 = OpCompositeConstruct %mat4v4float %162 %164 %166 %168
OpStore %x_4 %169
%170 = OpLoad %mat4v4float %negated_1
%172 = OpCompositeExtract %v4float %170 0
%173 = OpFOrdEqual %v4bool %162 %172
%174 = OpAll %bool %173
%175 = OpCompositeExtract %v4float %170 1
%176 = OpFOrdEqual %v4bool %164 %175
%177 = OpAll %bool %176
%178 = OpLogicalAnd %bool %174 %177
%179 = OpCompositeExtract %v4float %170 2
%180 = OpFOrdEqual %v4bool %166 %179
%181 = OpAll %bool %180
%182 = OpLogicalAnd %bool %178 %181
%183 = OpCompositeExtract %v4float %170 3
%184 = OpFOrdEqual %v4bool %168 %183
%185 = OpAll %bool %184
%186 = OpLogicalAnd %bool %182 %185
OpReturnValue %186
OpFunctionEnd
%main = OpFunction %v4float None %187
%188 = OpFunctionParameter %_ptr_Function_v2float
%189 = OpLabel
%_0_x = OpVariable %_ptr_Function_float Function
%223 = OpVariable %_ptr_Function_v4float Function
%192 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%193 = OpLoad %v4float %192
%194 = OpCompositeExtract %float %193 0
OpStore %_0_x %194
%195 = OpLoad %float %_0_x
%196 = OpFNegate %float %195
OpStore %_0_x %196
%198 = OpFOrdEqual %bool %196 %float_n1
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%201 = OpFunctionCall %bool %test_iscalar_b
OpBranch %200
%200 = OpLabel
%202 = OpPhi %bool %false %189 %201 %199
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpFunctionCall %bool %test_fvec_b
OpBranch %204
%204 = OpLabel
%206 = OpPhi %bool %false %200 %205 %203
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpFunctionCall %bool %test_ivec_b
OpBranch %208
%208 = OpLabel
%210 = OpPhi %bool %false %204 %209 %207
OpSelectionMerge %212 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%213 = OpFunctionCall %bool %test_mat2_b
OpBranch %212
%212 = OpLabel
%214 = OpPhi %bool %false %208 %213 %211
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%217 = OpFunctionCall %bool %test_mat3_b
OpBranch %216
%216 = OpLabel
%218 = OpPhi %bool %false %212 %217 %215
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpFunctionCall %bool %test_mat4_b
OpBranch %220
%220 = OpLabel
%222 = OpPhi %bool %false %216 %221 %219
OpSelectionMerge %227 None
OpBranchConditional %222 %225 %226
%225 = OpLabel
%228 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%230 = OpLoad %v4float %228
OpStore %223 %230
OpBranch %227
%226 = OpLabel
%231 = OpAccessChain %_ptr_Uniform_v4float %16 %int_2
%233 = OpLoad %v4float %231
OpStore %223 %233
OpBranch %227
%227 = OpLabel
%234 = OpLoad %v4float %223
OpReturnValue %234
OpFunctionEnd
