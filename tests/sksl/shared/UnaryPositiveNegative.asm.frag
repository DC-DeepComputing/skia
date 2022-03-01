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
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %x_0 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%29 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%float_n1 = OpConstant %float -1
%54 = OpConstantComposite %v2float %float_n1 %float_n1
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%70 = OpConstantComposite %v2int %int_n1 %int_n1
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%v4bool = OpTypeVector %bool 4
%190 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%test_iscalar_b = OpFunction %bool None %29
%30 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%34 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%37 = OpLoad %v4float %34
%38 = OpCompositeExtract %float %37 0
%39 = OpConvertFToS %int %38
OpStore %x %39
%40 = OpLoad %int %x
%41 = OpSNegate %int %40
OpStore %x %41
%42 = OpLoad %int %x
%44 = OpIEqual %bool %42 %int_n1
OpReturnValue %44
OpFunctionEnd
%test_fvec_b = OpFunction %bool None %29
%45 = OpLabel
%x_0 = OpVariable %_ptr_Function_v2float Function
%47 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%48 = OpLoad %v4float %47
%49 = OpVectorShuffle %v2float %48 %48 0 1
OpStore %x_0 %49
%50 = OpLoad %v2float %x_0
%51 = OpFNegate %v2float %50
OpStore %x_0 %51
%52 = OpLoad %v2float %x_0
%55 = OpFOrdEqual %v2bool %52 %54
%57 = OpAll %bool %55
OpReturnValue %57
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %29
%58 = OpLabel
%x_1 = OpVariable %_ptr_Function_v2int Function
%62 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%63 = OpLoad %v4float %62
%64 = OpCompositeExtract %float %63 0
%65 = OpConvertFToS %int %64
%66 = OpCompositeConstruct %v2int %65 %65
OpStore %x_1 %66
%67 = OpLoad %v2int %x_1
%68 = OpSNegate %v2int %67
OpStore %x_1 %68
%69 = OpLoad %v2int %x_1
%71 = OpIEqual %v2bool %69 %70
%72 = OpAll %bool %71
OpReturnValue %72
OpFunctionEnd
%test_mat2_b = OpFunction %bool None %29
%73 = OpLabel
%x_2 = OpVariable %_ptr_Function_mat2v2float Function
%77 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%78 = OpLoad %v4float %77
%79 = OpCompositeExtract %float %78 0
%81 = OpCompositeConstruct %v2float %79 %float_0
%82 = OpCompositeConstruct %v2float %float_0 %79
%80 = OpCompositeConstruct %mat2v2float %81 %82
OpStore %x_2 %80
%83 = OpLoad %mat2v2float %x_2
%84 = OpCompositeExtract %v2float %83 0
%85 = OpFNegate %v2float %84
%86 = OpCompositeExtract %v2float %83 1
%87 = OpFNegate %v2float %86
%88 = OpCompositeConstruct %mat2v2float %85 %87
OpStore %x_2 %88
%89 = OpLoad %mat2v2float %x_2
%91 = OpCompositeConstruct %v2float %float_n1 %float_0
%92 = OpCompositeConstruct %v2float %float_0 %float_n1
%90 = OpCompositeConstruct %mat2v2float %91 %92
%93 = OpCompositeExtract %v2float %89 0
%94 = OpCompositeExtract %v2float %90 0
%95 = OpFOrdEqual %v2bool %93 %94
%96 = OpAll %bool %95
%97 = OpCompositeExtract %v2float %89 1
%98 = OpCompositeExtract %v2float %90 1
%99 = OpFOrdEqual %v2bool %97 %98
%100 = OpAll %bool %99
%101 = OpLogicalAnd %bool %96 %100
OpReturnValue %101
OpFunctionEnd
%test_mat3_b = OpFunction %bool None %29
%102 = OpLabel
%x_3 = OpVariable %_ptr_Function_mat3v3float Function
%107 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%108 = OpLoad %v4float %107
%109 = OpCompositeExtract %float %108 0
%111 = OpCompositeConstruct %v3float %109 %float_0 %float_0
%112 = OpCompositeConstruct %v3float %float_0 %109 %float_0
%113 = OpCompositeConstruct %v3float %float_0 %float_0 %109
%110 = OpCompositeConstruct %mat3v3float %111 %112 %113
OpStore %x_3 %110
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
%124 = OpCompositeConstruct %v3float %float_n1 %float_0 %float_0
%125 = OpCompositeConstruct %v3float %float_0 %float_n1 %float_0
%126 = OpCompositeConstruct %v3float %float_0 %float_0 %float_n1
%123 = OpCompositeConstruct %mat3v3float %124 %125 %126
%128 = OpCompositeExtract %v3float %122 0
%129 = OpCompositeExtract %v3float %123 0
%130 = OpFOrdEqual %v3bool %128 %129
%131 = OpAll %bool %130
%132 = OpCompositeExtract %v3float %122 1
%133 = OpCompositeExtract %v3float %123 1
%134 = OpFOrdEqual %v3bool %132 %133
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %131 %135
%137 = OpCompositeExtract %v3float %122 2
%138 = OpCompositeExtract %v3float %123 2
%139 = OpFOrdEqual %v3bool %137 %138
%140 = OpAll %bool %139
%141 = OpLogicalAnd %bool %136 %140
OpReturnValue %141
OpFunctionEnd
%test_mat4_b = OpFunction %bool None %29
%142 = OpLabel
%x_4 = OpVariable %_ptr_Function_mat4v4float Function
%146 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%147 = OpLoad %v4float %146
%148 = OpCompositeExtract %float %147 0
%150 = OpCompositeConstruct %v4float %148 %float_0 %float_0 %float_0
%151 = OpCompositeConstruct %v4float %float_0 %148 %float_0 %float_0
%152 = OpCompositeConstruct %v4float %float_0 %float_0 %148 %float_0
%153 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %148
%149 = OpCompositeConstruct %mat4v4float %150 %151 %152 %153
OpStore %x_4 %149
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
%166 = OpCompositeConstruct %v4float %float_n1 %float_0 %float_0 %float_0
%167 = OpCompositeConstruct %v4float %float_0 %float_n1 %float_0 %float_0
%168 = OpCompositeConstruct %v4float %float_0 %float_0 %float_n1 %float_0
%169 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_n1
%165 = OpCompositeConstruct %mat4v4float %166 %167 %168 %169
%171 = OpCompositeExtract %v4float %164 0
%172 = OpCompositeExtract %v4float %165 0
%173 = OpFOrdEqual %v4bool %171 %172
%174 = OpAll %bool %173
%175 = OpCompositeExtract %v4float %164 1
%176 = OpCompositeExtract %v4float %165 1
%177 = OpFOrdEqual %v4bool %175 %176
%178 = OpAll %bool %177
%179 = OpLogicalAnd %bool %174 %178
%180 = OpCompositeExtract %v4float %164 2
%181 = OpCompositeExtract %v4float %165 2
%182 = OpFOrdEqual %v4bool %180 %181
%183 = OpAll %bool %182
%184 = OpLogicalAnd %bool %179 %183
%185 = OpCompositeExtract %v4float %164 3
%186 = OpCompositeExtract %v4float %165 3
%187 = OpFOrdEqual %v4bool %185 %186
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %184 %188
OpReturnValue %189
OpFunctionEnd
%main = OpFunction %v4float None %190
%191 = OpFunctionParameter %_ptr_Function_v2float
%192 = OpLabel
%_0_x = OpVariable %_ptr_Function_float Function
%227 = OpVariable %_ptr_Function_v4float Function
%195 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%196 = OpLoad %v4float %195
%197 = OpCompositeExtract %float %196 0
OpStore %_0_x %197
%198 = OpLoad %float %_0_x
%199 = OpFNegate %float %198
OpStore %_0_x %199
%201 = OpLoad %float %_0_x
%202 = OpFOrdEqual %bool %201 %float_n1
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpFunctionCall %bool %test_iscalar_b
OpBranch %204
%204 = OpLabel
%206 = OpPhi %bool %false %192 %205 %203
OpSelectionMerge %208 None
OpBranchConditional %206 %207 %208
%207 = OpLabel
%209 = OpFunctionCall %bool %test_fvec_b
OpBranch %208
%208 = OpLabel
%210 = OpPhi %bool %false %204 %209 %207
OpSelectionMerge %212 None
OpBranchConditional %210 %211 %212
%211 = OpLabel
%213 = OpFunctionCall %bool %test_ivec_b
OpBranch %212
%212 = OpLabel
%214 = OpPhi %bool %false %208 %213 %211
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%217 = OpFunctionCall %bool %test_mat2_b
OpBranch %216
%216 = OpLabel
%218 = OpPhi %bool %false %212 %217 %215
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%221 = OpFunctionCall %bool %test_mat3_b
OpBranch %220
%220 = OpLabel
%222 = OpPhi %bool %false %216 %221 %219
OpSelectionMerge %224 None
OpBranchConditional %222 %223 %224
%223 = OpLabel
%225 = OpFunctionCall %bool %test_mat4_b
OpBranch %224
%224 = OpLabel
%226 = OpPhi %bool %false %220 %225 %223
OpSelectionMerge %231 None
OpBranchConditional %226 %229 %230
%229 = OpLabel
%232 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%234 = OpLoad %v4float %232
OpStore %227 %234
OpBranch %231
%230 = OpLabel
%235 = OpAccessChain %_ptr_Uniform_v4float %16 %int_2
%237 = OpLoad %v4float %235
OpStore %227 %237
OpBranch %231
%231 = OpLabel
%238 = OpLoad %v4float %227
OpReturnValue %238
OpFunctionEnd
