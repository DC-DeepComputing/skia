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
OpDecorate %69 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
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
%59 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%60 = OpConstantComposite %v3float %float_4 %float_1 %float_2
%61 = OpConstantComposite %mat2v3float %59 %60
%v3bool = OpTypeVector %bool 3
%false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
%89 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%90 = OpConstantComposite %mat2v4float %89 %89
%v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
%120 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%121 = OpConstantComposite %mat3v3float %59 %60 %120
%mat4v2float = OpTypeMatrix %v2float 4
%152 = OpConstantComposite %v2float %float_1 %float_2
%153 = OpConstantComposite %v2float %float_3 %float_4
%154 = OpConstantComposite %mat4v2float %152 %153 %152 %153
%v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
%191 = OpConstantComposite %v3float %float_2 %float_3 %float_4
%192 = OpConstantComposite %mat4v3float %59 %60 %120 %191
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
%205 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpLoad %v4float %f4
%42 = OpLoad %v4float %f4
%43 = OpVectorShuffle %v2float %42 %42 0 1
%44 = OpCompositeExtract %float %41 0
%45 = OpCompositeExtract %float %41 1
%46 = OpCompositeExtract %float %41 2
%48 = OpCompositeConstruct %v3float %44 %45 %46
%49 = OpCompositeExtract %float %41 3
%50 = OpCompositeExtract %float %43 0
%51 = OpCompositeExtract %float %43 1
%52 = OpCompositeConstruct %v3float %49 %50 %51
%54 = OpCompositeConstruct %mat2v3float %48 %52
%63 = OpFOrdEqual %v3bool %48 %59
%64 = OpAll %bool %63
%65 = OpFOrdEqual %v3bool %52 %60
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %64 %66
OpStore %ok %67
%69 = OpLoad %bool %ok
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpLoad %v4float %f4
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%74 = OpLoad %v4float %f4
%75 = OpVectorShuffle %v4float %74 %74 3 0 1 2
%76 = OpLoad %v4float %f4
%77 = OpCompositeExtract %float %76 3
%78 = OpCompositeExtract %float %73 0
%79 = OpCompositeExtract %float %73 1
%80 = OpCompositeExtract %float %73 2
%81 = OpCompositeExtract %float %75 0
%82 = OpCompositeConstruct %v4float %78 %79 %80 %81
%83 = OpCompositeExtract %float %75 1
%84 = OpCompositeExtract %float %75 2
%85 = OpCompositeExtract %float %75 3
%86 = OpCompositeConstruct %v4float %83 %84 %85 %77
%88 = OpCompositeConstruct %mat2v4float %82 %86
%92 = OpFOrdEqual %v4bool %82 %89
%93 = OpAll %bool %92
%94 = OpFOrdEqual %v4bool %86 %89
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %93 %95
OpBranch %71
%71 = OpLabel
%97 = OpPhi %bool %false %26 %96 %70
OpStore %ok %97
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpLoad %v4float %f4
%101 = OpVectorShuffle %v2float %100 %100 0 1
%102 = OpLoad %v4float %f4
%103 = OpVectorShuffle %v2float %102 %102 2 3
%104 = OpLoad %v4float %f4
%105 = OpLoad %v4float %f4
%106 = OpCompositeExtract %float %105 0
%107 = OpCompositeExtract %float %101 0
%108 = OpCompositeExtract %float %101 1
%109 = OpCompositeExtract %float %103 0
%110 = OpCompositeConstruct %v3float %107 %108 %109
%111 = OpCompositeExtract %float %103 1
%112 = OpCompositeExtract %float %104 0
%113 = OpCompositeExtract %float %104 1
%114 = OpCompositeConstruct %v3float %111 %112 %113
%115 = OpCompositeExtract %float %104 2
%116 = OpCompositeExtract %float %104 3
%117 = OpCompositeConstruct %v3float %115 %116 %106
%119 = OpCompositeConstruct %mat3v3float %110 %114 %117
%122 = OpFOrdEqual %v3bool %110 %59
%123 = OpAll %bool %122
%124 = OpFOrdEqual %v3bool %114 %60
%125 = OpAll %bool %124
%126 = OpLogicalAnd %bool %123 %125
%127 = OpFOrdEqual %v3bool %117 %120
%128 = OpAll %bool %127
%129 = OpLogicalAnd %bool %126 %128
OpBranch %99
%99 = OpLabel
%130 = OpPhi %bool %false %71 %129 %98
OpStore %ok %130
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %v4float %f4
%134 = OpVectorShuffle %v3float %133 %133 0 1 2
%135 = OpLoad %v4float %f4
%136 = OpVectorShuffle %v4float %135 %135 3 0 1 2
%137 = OpLoad %v4float %f4
%138 = OpCompositeExtract %float %137 3
%139 = OpCompositeExtract %float %134 0
%140 = OpCompositeExtract %float %134 1
%141 = OpCompositeConstruct %v2float %139 %140
%142 = OpCompositeExtract %float %134 2
%143 = OpCompositeExtract %float %136 0
%144 = OpCompositeConstruct %v2float %142 %143
%145 = OpCompositeExtract %float %136 1
%146 = OpCompositeExtract %float %136 2
%147 = OpCompositeConstruct %v2float %145 %146
%148 = OpCompositeExtract %float %136 3
%149 = OpCompositeConstruct %v2float %148 %138
%151 = OpCompositeConstruct %mat4v2float %141 %144 %147 %149
%156 = OpFOrdEqual %v2bool %141 %152
%157 = OpAll %bool %156
%158 = OpFOrdEqual %v2bool %144 %153
%159 = OpAll %bool %158
%160 = OpLogicalAnd %bool %157 %159
%161 = OpFOrdEqual %v2bool %147 %152
%162 = OpAll %bool %161
%163 = OpLogicalAnd %bool %160 %162
%164 = OpFOrdEqual %v2bool %149 %153
%165 = OpAll %bool %164
%166 = OpLogicalAnd %bool %163 %165
OpBranch %132
%132 = OpLabel
%167 = OpPhi %bool %false %99 %166 %131
OpStore %ok %167
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpLoad %v4float %f4
%171 = OpCompositeExtract %float %170 0
%172 = OpLoad %v4float %f4
%173 = OpVectorShuffle %v4float %172 %172 1 2 3 0
%174 = OpLoad %v4float %f4
%175 = OpVectorShuffle %v4float %174 %174 1 2 3 0
%176 = OpLoad %v4float %f4
%177 = OpVectorShuffle %v3float %176 %176 1 2 3
%178 = OpCompositeExtract %float %173 0
%179 = OpCompositeExtract %float %173 1
%180 = OpCompositeConstruct %v3float %171 %178 %179
%181 = OpCompositeExtract %float %173 2
%182 = OpCompositeExtract %float %173 3
%183 = OpCompositeExtract %float %175 0
%184 = OpCompositeConstruct %v3float %181 %182 %183
%185 = OpCompositeExtract %float %175 1
%186 = OpCompositeExtract %float %175 2
%187 = OpCompositeExtract %float %175 3
%188 = OpCompositeConstruct %v3float %185 %186 %187
%190 = OpCompositeConstruct %mat4v3float %180 %184 %188 %177
%193 = OpFOrdEqual %v3bool %180 %59
%194 = OpAll %bool %193
%195 = OpFOrdEqual %v3bool %184 %60
%196 = OpAll %bool %195
%197 = OpLogicalAnd %bool %194 %196
%198 = OpFOrdEqual %v3bool %188 %120
%199 = OpAll %bool %198
%200 = OpLogicalAnd %bool %197 %199
%201 = OpFOrdEqual %v3bool %177 %191
%202 = OpAll %bool %201
%203 = OpLogicalAnd %bool %200 %202
OpBranch %169
%169 = OpLabel
%204 = OpPhi %bool %false %132 %203 %168
OpStore %ok %204
OpSelectionMerge %208 None
OpBranchConditional %204 %206 %207
%206 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%212 = OpLoad %v4float %209
OpStore %205 %212
OpBranch %208
%207 = OpLabel
%213 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%215 = OpLoad %v4float %213
OpStore %205 %215
OpBranch %208
%208 = OpLabel
%216 = OpLoad %v4float %205
OpReturnValue %216
OpFunctionEnd
