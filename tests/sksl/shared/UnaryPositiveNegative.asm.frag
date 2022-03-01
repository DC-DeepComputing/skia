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
OpName %x_2 "x"
OpName %test_mat3_b "test_mat3_b"
OpName %x_3 "x"
OpName %test_mat4_b "test_mat4_b"
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
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %243 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
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
%58 = OpConstantComposite %v2float %float_n1 %float_n1
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%74 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_4 = OpConstant %int 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_5 = OpConstant %int 5
%v4bool = OpTypeVector %bool 4
%196 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%46 = OpLoad %int %x
%48 = OpIEqual %bool %46 %int_n1
OpReturnValue %48
OpFunctionEnd
%test_fvec_b = OpFunction %bool None %33
%49 = OpLabel
%x_0 = OpVariable %_ptr_Function_v2float Function
%51 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
OpStore %x_0 %53
%54 = OpLoad %v2float %x_0
%55 = OpFNegate %v2float %54
OpStore %x_0 %55
%56 = OpLoad %v2float %x_0
%59 = OpFOrdEqual %v2bool %56 %58
%61 = OpAll %bool %59
OpReturnValue %61
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %33
%62 = OpLabel
%x_1 = OpVariable %_ptr_Function_v2int Function
%66 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%67 = OpLoad %v4float %66
%68 = OpCompositeExtract %float %67 0
%69 = OpConvertFToS %int %68
%70 = OpCompositeConstruct %v2int %69 %69
OpStore %x_1 %70
%71 = OpLoad %v2int %x_1
%72 = OpSNegate %v2int %71
OpStore %x_1 %72
%73 = OpLoad %v2int %x_1
%75 = OpIEqual %v2bool %73 %74
%76 = OpAll %bool %75
OpReturnValue %76
OpFunctionEnd
%test_mat2_b = OpFunction %bool None %33
%77 = OpLabel
%x_2 = OpVariable %_ptr_Function_mat2v2float Function
%80 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
%83 = OpLoad %mat2v2float %80
OpStore %x_2 %83
%84 = OpLoad %mat2v2float %x_2
%85 = OpCompositeExtract %v2float %84 0
%86 = OpFNegate %v2float %85
%87 = OpCompositeExtract %v2float %84 1
%88 = OpFNegate %v2float %87
%89 = OpCompositeConstruct %mat2v2float %86 %88
OpStore %x_2 %89
%90 = OpLoad %mat2v2float %x_2
%91 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
%92 = OpLoad %mat2v2float %91
%93 = OpCompositeExtract %v2float %92 0
%94 = OpFNegate %v2float %93
%95 = OpCompositeExtract %v2float %92 1
%96 = OpFNegate %v2float %95
%97 = OpCompositeConstruct %mat2v2float %94 %96
%98 = OpCompositeExtract %v2float %90 0
%99 = OpCompositeExtract %v2float %97 0
%100 = OpFOrdEqual %v2bool %98 %99
%101 = OpAll %bool %100
%102 = OpCompositeExtract %v2float %90 1
%103 = OpCompositeExtract %v2float %97 1
%104 = OpFOrdEqual %v2bool %102 %103
%105 = OpAll %bool %104
%106 = OpLogicalAnd %bool %101 %105
OpReturnValue %106
OpFunctionEnd
%test_mat3_b = OpFunction %bool None %33
%107 = OpLabel
%x_3 = OpVariable %_ptr_Function_mat3v3float Function
%110 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
%113 = OpLoad %mat3v3float %110
OpStore %x_3 %113
%114 = OpLoad %mat3v3float %x_3
%115 = OpCompositeExtract %v3float %114 0
%116 = OpFNegate %v3float %115
%117 = OpCompositeExtract %v3float %114 1
%118 = OpFNegate %v3float %117
%119 = OpCompositeExtract %v3float %114 2
%120 = OpFNegate %v3float %119
%121 = OpCompositeConstruct %mat3v3float %116 %118 %120
OpStore %x_3 %121
%122 = OpLoad %mat3v3float %x_3
%123 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
%124 = OpLoad %mat3v3float %123
%125 = OpCompositeExtract %v3float %124 0
%126 = OpFNegate %v3float %125
%127 = OpCompositeExtract %v3float %124 1
%128 = OpFNegate %v3float %127
%129 = OpCompositeExtract %v3float %124 2
%130 = OpFNegate %v3float %129
%131 = OpCompositeConstruct %mat3v3float %126 %128 %130
%133 = OpCompositeExtract %v3float %122 0
%134 = OpCompositeExtract %v3float %131 0
%135 = OpFOrdEqual %v3bool %133 %134
%136 = OpAll %bool %135
%137 = OpCompositeExtract %v3float %122 1
%138 = OpCompositeExtract %v3float %131 1
%139 = OpFOrdEqual %v3bool %137 %138
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %136 %140
%142 = OpCompositeExtract %v3float %122 2
%143 = OpCompositeExtract %v3float %131 2
%144 = OpFOrdEqual %v3bool %142 %143
%145 = OpAll %bool %144
%146 = OpLogicalAnd %bool %141 %145
OpReturnValue %146
OpFunctionEnd
%test_mat4_b = OpFunction %bool None %33
%147 = OpLabel
%x_4 = OpVariable %_ptr_Function_mat4v4float Function
%150 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
%153 = OpLoad %mat4v4float %150
OpStore %x_4 %153
%154 = OpLoad %mat4v4float %x_4
%155 = OpCompositeExtract %v4float %154 0
%156 = OpFNegate %v4float %155
%157 = OpCompositeExtract %v4float %154 1
%158 = OpFNegate %v4float %157
%159 = OpCompositeExtract %v4float %154 2
%160 = OpFNegate %v4float %159
%161 = OpCompositeExtract %v4float %154 3
%162 = OpFNegate %v4float %161
%163 = OpCompositeConstruct %mat4v4float %156 %158 %160 %162
OpStore %x_4 %163
%164 = OpLoad %mat4v4float %x_4
%165 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
%166 = OpLoad %mat4v4float %165
%167 = OpCompositeExtract %v4float %166 0
%168 = OpFNegate %v4float %167
%169 = OpCompositeExtract %v4float %166 1
%170 = OpFNegate %v4float %169
%171 = OpCompositeExtract %v4float %166 2
%172 = OpFNegate %v4float %171
%173 = OpCompositeExtract %v4float %166 3
%174 = OpFNegate %v4float %173
%175 = OpCompositeConstruct %mat4v4float %168 %170 %172 %174
%177 = OpCompositeExtract %v4float %164 0
%178 = OpCompositeExtract %v4float %175 0
%179 = OpFOrdEqual %v4bool %177 %178
%180 = OpAll %bool %179
%181 = OpCompositeExtract %v4float %164 1
%182 = OpCompositeExtract %v4float %175 1
%183 = OpFOrdEqual %v4bool %181 %182
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %180 %184
%186 = OpCompositeExtract %v4float %164 2
%187 = OpCompositeExtract %v4float %175 2
%188 = OpFOrdEqual %v4bool %186 %187
%189 = OpAll %bool %188
%190 = OpLogicalAnd %bool %185 %189
%191 = OpCompositeExtract %v4float %164 3
%192 = OpCompositeExtract %v4float %175 3
%193 = OpFOrdEqual %v4bool %191 %192
%194 = OpAll %bool %193
%195 = OpLogicalAnd %bool %190 %194
OpReturnValue %195
OpFunctionEnd
%main = OpFunction %v4float None %196
%197 = OpFunctionParameter %_ptr_Function_v2float
%198 = OpLabel
%_0_x = OpVariable %_ptr_Function_float Function
%233 = OpVariable %_ptr_Function_v4float Function
%201 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%202 = OpLoad %v4float %201
%203 = OpCompositeExtract %float %202 0
OpStore %_0_x %203
%204 = OpLoad %float %_0_x
%205 = OpFNegate %float %204
OpStore %_0_x %205
%207 = OpLoad %float %_0_x
%208 = OpFOrdEqual %bool %207 %float_n1
OpSelectionMerge %210 None
OpBranchConditional %208 %209 %210
%209 = OpLabel
%211 = OpFunctionCall %bool %test_iscalar_b
OpBranch %210
%210 = OpLabel
%212 = OpPhi %bool %false %198 %211 %209
OpSelectionMerge %214 None
OpBranchConditional %212 %213 %214
%213 = OpLabel
%215 = OpFunctionCall %bool %test_fvec_b
OpBranch %214
%214 = OpLabel
%216 = OpPhi %bool %false %210 %215 %213
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%219 = OpFunctionCall %bool %test_ivec_b
OpBranch %218
%218 = OpLabel
%220 = OpPhi %bool %false %214 %219 %217
OpSelectionMerge %222 None
OpBranchConditional %220 %221 %222
%221 = OpLabel
%223 = OpFunctionCall %bool %test_mat2_b
OpBranch %222
%222 = OpLabel
%224 = OpPhi %bool %false %218 %223 %221
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%227 = OpFunctionCall %bool %test_mat3_b
OpBranch %226
%226 = OpLabel
%228 = OpPhi %bool %false %222 %227 %225
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpFunctionCall %bool %test_mat4_b
OpBranch %230
%230 = OpLabel
%232 = OpPhi %bool %false %226 %231 %229
OpSelectionMerge %237 None
OpBranchConditional %232 %235 %236
%235 = OpLabel
%238 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%240 = OpLoad %v4float %238
OpStore %233 %240
OpBranch %237
%236 = OpLabel
%241 = OpAccessChain %_ptr_Uniform_v4float %16 %int_2
%243 = OpLoad %v4float %241
OpStore %233 %243
OpBranch %237
%237 = OpLabel
%244 = OpLoad %v4float %233
OpReturnValue %244
OpFunctionEnd
