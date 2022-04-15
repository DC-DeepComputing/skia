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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %205 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%53 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%54 = OpConstantComposite %v3float %float_4 %float_1 %float_2
%55 = OpConstantComposite %mat2v3float %53 %54
%v3bool = OpTypeVector %bool 3
%false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
%82 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%83 = OpConstantComposite %mat2v4float %82 %82
%v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
%113 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%114 = OpConstantComposite %mat3v3float %53 %54 %113
%mat4v2float = OpTypeMatrix %v2float 4
%145 = OpConstantComposite %v2float %float_1 %float_2
%146 = OpConstantComposite %v2float %float_3 %float_4
%147 = OpConstantComposite %mat4v2float %145 %146 %145 %146
%v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
%184 = OpConstantComposite %v3float %float_2 %float_3 %float_4
%185 = OpConstantComposite %mat4v3float %53 %54 %113 %184
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%198 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpVectorShuffle %v2float %38 %38 0 1
%43 = OpCompositeConstruct %v3float %34 %35 %36
%44 = OpCompositeExtract %float %41 0
%45 = OpCompositeExtract %float %41 1
%46 = OpCompositeConstruct %v3float %37 %44 %45
%48 = OpCompositeConstruct %mat2v3float %43 %46
%57 = OpFOrdEqual %v3bool %43 %53
%58 = OpAll %bool %57
%59 = OpFOrdEqual %v3bool %46 %54
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %58 %60
OpStore %ok %61
OpSelectionMerge %64 None
OpBranchConditional %61 %63 %64
%63 = OpLabel
%65 = OpLoad %v4float %f4
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%67 = OpLoad %v4float %f4
%68 = OpVectorShuffle %v4float %67 %67 3 0 1 2
%69 = OpLoad %v4float %f4
%70 = OpCompositeExtract %float %69 3
%71 = OpCompositeExtract %float %66 0
%72 = OpCompositeExtract %float %66 1
%73 = OpCompositeExtract %float %66 2
%74 = OpCompositeExtract %float %68 0
%75 = OpCompositeConstruct %v4float %71 %72 %73 %74
%76 = OpCompositeExtract %float %68 1
%77 = OpCompositeExtract %float %68 2
%78 = OpCompositeExtract %float %68 3
%79 = OpCompositeConstruct %v4float %76 %77 %78 %70
%81 = OpCompositeConstruct %mat2v4float %75 %79
%85 = OpFOrdEqual %v4bool %75 %82
%86 = OpAll %bool %85
%87 = OpFOrdEqual %v4bool %79 %82
%88 = OpAll %bool %87
%89 = OpLogicalAnd %bool %86 %88
OpBranch %64
%64 = OpLabel
%90 = OpPhi %bool %false %26 %89 %63
OpStore %ok %90
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %v4float %f4
%94 = OpVectorShuffle %v2float %93 %93 0 1
%95 = OpLoad %v4float %f4
%96 = OpVectorShuffle %v2float %95 %95 2 3
%97 = OpLoad %v4float %f4
%98 = OpLoad %v4float %f4
%99 = OpCompositeExtract %float %98 0
%100 = OpCompositeExtract %float %94 0
%101 = OpCompositeExtract %float %94 1
%102 = OpCompositeExtract %float %96 0
%103 = OpCompositeConstruct %v3float %100 %101 %102
%104 = OpCompositeExtract %float %96 1
%105 = OpCompositeExtract %float %97 0
%106 = OpCompositeExtract %float %97 1
%107 = OpCompositeConstruct %v3float %104 %105 %106
%108 = OpCompositeExtract %float %97 2
%109 = OpCompositeExtract %float %97 3
%110 = OpCompositeConstruct %v3float %108 %109 %99
%112 = OpCompositeConstruct %mat3v3float %103 %107 %110
%115 = OpFOrdEqual %v3bool %103 %53
%116 = OpAll %bool %115
%117 = OpFOrdEqual %v3bool %107 %54
%118 = OpAll %bool %117
%119 = OpLogicalAnd %bool %116 %118
%120 = OpFOrdEqual %v3bool %110 %113
%121 = OpAll %bool %120
%122 = OpLogicalAnd %bool %119 %121
OpBranch %92
%92 = OpLabel
%123 = OpPhi %bool %false %64 %122 %91
OpStore %ok %123
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpLoad %v4float %f4
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%128 = OpLoad %v4float %f4
%129 = OpVectorShuffle %v4float %128 %128 3 0 1 2
%130 = OpLoad %v4float %f4
%131 = OpCompositeExtract %float %130 3
%132 = OpCompositeExtract %float %127 0
%133 = OpCompositeExtract %float %127 1
%134 = OpCompositeConstruct %v2float %132 %133
%135 = OpCompositeExtract %float %127 2
%136 = OpCompositeExtract %float %129 0
%137 = OpCompositeConstruct %v2float %135 %136
%138 = OpCompositeExtract %float %129 1
%139 = OpCompositeExtract %float %129 2
%140 = OpCompositeConstruct %v2float %138 %139
%141 = OpCompositeExtract %float %129 3
%142 = OpCompositeConstruct %v2float %141 %131
%144 = OpCompositeConstruct %mat4v2float %134 %137 %140 %142
%149 = OpFOrdEqual %v2bool %134 %145
%150 = OpAll %bool %149
%151 = OpFOrdEqual %v2bool %137 %146
%152 = OpAll %bool %151
%153 = OpLogicalAnd %bool %150 %152
%154 = OpFOrdEqual %v2bool %140 %145
%155 = OpAll %bool %154
%156 = OpLogicalAnd %bool %153 %155
%157 = OpFOrdEqual %v2bool %142 %146
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %156 %158
OpBranch %125
%125 = OpLabel
%160 = OpPhi %bool %false %92 %159 %124
OpStore %ok %160
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpLoad %v4float %f4
%164 = OpCompositeExtract %float %163 0
%165 = OpLoad %v4float %f4
%166 = OpVectorShuffle %v4float %165 %165 1 2 3 0
%167 = OpLoad %v4float %f4
%168 = OpVectorShuffle %v4float %167 %167 1 2 3 0
%169 = OpLoad %v4float %f4
%170 = OpVectorShuffle %v3float %169 %169 1 2 3
%171 = OpCompositeExtract %float %166 0
%172 = OpCompositeExtract %float %166 1
%173 = OpCompositeConstruct %v3float %164 %171 %172
%174 = OpCompositeExtract %float %166 2
%175 = OpCompositeExtract %float %166 3
%176 = OpCompositeExtract %float %168 0
%177 = OpCompositeConstruct %v3float %174 %175 %176
%178 = OpCompositeExtract %float %168 1
%179 = OpCompositeExtract %float %168 2
%180 = OpCompositeExtract %float %168 3
%181 = OpCompositeConstruct %v3float %178 %179 %180
%183 = OpCompositeConstruct %mat4v3float %173 %177 %181 %170
%186 = OpFOrdEqual %v3bool %173 %53
%187 = OpAll %bool %186
%188 = OpFOrdEqual %v3bool %177 %54
%189 = OpAll %bool %188
%190 = OpLogicalAnd %bool %187 %189
%191 = OpFOrdEqual %v3bool %181 %113
%192 = OpAll %bool %191
%193 = OpLogicalAnd %bool %190 %192
%194 = OpFOrdEqual %v3bool %170 %184
%195 = OpAll %bool %194
%196 = OpLogicalAnd %bool %193 %195
OpBranch %162
%162 = OpLabel
%197 = OpPhi %bool %false %125 %196 %161
OpStore %ok %197
OpSelectionMerge %201 None
OpBranchConditional %197 %199 %200
%199 = OpLabel
%202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%205 = OpLoad %v4float %202
OpStore %198 %205
OpBranch %201
%200 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%208 = OpLoad %v4float %206
OpStore %198 %208
OpBranch %201
%201 = OpLabel
%209 = OpLoad %v4float %198
OpReturnValue %209
OpFunctionEnd
