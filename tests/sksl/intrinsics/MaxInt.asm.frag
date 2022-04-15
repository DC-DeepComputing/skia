OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %intValues "intValues"
OpName %intGreen "intGreen"
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
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
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%int_50 = OpConstant %int 50
%int_75 = OpConstant %int 75
%int_225 = OpConstant %int 225
%63 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
%int_100 = OpConstant %int 100
%66 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%77 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%90 = OpConstantComposite %v3int %int_50 %int_50 %int_50
%v3bool = OpTypeVector %bool 3
%101 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%122 = OpConstantComposite %v3int %int_50 %int_50 %int_75
%186 = OpConstantComposite %v2int %int_0 %int_100
%194 = OpConstantComposite %v3int %int_0 %int_100 %int_75
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_2 = OpConstant %int 2
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
%intValues = OpVariable %_ptr_Function_v4int Function
%intGreen = OpVariable %_ptr_Function_v4int Function
%expectedA = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%206 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %30
%35 = OpVectorTimesScalar %v4float %33 %float_100
%36 = OpCompositeExtract %float %35 0
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %35 1
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %35 2
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %35 3
%43 = OpConvertFToS %int %42
%44 = OpCompositeConstruct %v4int %37 %39 %41 %43
OpStore %intValues %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%48 = OpLoad %v4float %46
%49 = OpVectorTimesScalar %v4float %48 %float_100
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %intGreen %58
OpStore %expectedA %63
OpStore %expectedB %66
%69 = OpCompositeExtract %int %44 0
%68 = OpExtInst %int %1 SMax %69 %int_50
%70 = OpIEqual %bool %68 %int_50
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%74 = OpLoad %v4int %intValues
%75 = OpVectorShuffle %v2int %74 %74 0 1
%73 = OpExtInst %v2int %1 SMax %75 %77
%78 = OpLoad %v4int %expectedA
%79 = OpVectorShuffle %v2int %78 %78 0 1
%80 = OpIEqual %v2bool %73 %79
%82 = OpAll %bool %80
OpBranch %72
%72 = OpLabel
%83 = OpPhi %bool %false %25 %82 %71
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpLoad %v4int %intValues
%88 = OpVectorShuffle %v3int %87 %87 0 1 2
%86 = OpExtInst %v3int %1 SMax %88 %90
%91 = OpLoad %v4int %expectedA
%92 = OpVectorShuffle %v3int %91 %91 0 1 2
%93 = OpIEqual %v3bool %86 %92
%95 = OpAll %bool %93
OpBranch %85
%85 = OpLabel
%96 = OpPhi %bool %false %72 %95 %84
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpLoad %v4int %intValues
%99 = OpExtInst %v4int %1 SMax %100 %101
%102 = OpLoad %v4int %expectedA
%103 = OpIEqual %v4bool %99 %102
%105 = OpAll %bool %103
OpBranch %98
%98 = OpLabel
%106 = OpPhi %bool %false %85 %105 %97
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4int %expectedA
%110 = OpCompositeExtract %int %109 0
%111 = OpIEqual %bool %int_50 %110
OpBranch %108
%108 = OpLabel
%112 = OpPhi %bool %false %98 %111 %107
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%115 = OpLoad %v4int %expectedA
%116 = OpVectorShuffle %v2int %115 %115 0 1
%117 = OpIEqual %v2bool %77 %116
%118 = OpAll %bool %117
OpBranch %114
%114 = OpLabel
%119 = OpPhi %bool %false %108 %118 %113
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%123 = OpLoad %v4int %expectedA
%124 = OpVectorShuffle %v3int %123 %123 0 1 2
%125 = OpIEqual %v3bool %122 %124
%126 = OpAll %bool %125
OpBranch %121
%121 = OpLabel
%127 = OpPhi %bool %false %114 %126 %120
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpLoad %v4int %expectedA
%131 = OpIEqual %v4bool %63 %130
%132 = OpAll %bool %131
OpBranch %129
%129 = OpLabel
%133 = OpPhi %bool %false %121 %132 %128
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpLoad %v4int %intValues
%138 = OpCompositeExtract %int %137 0
%139 = OpLoad %v4int %intGreen
%140 = OpCompositeExtract %int %139 0
%136 = OpExtInst %int %1 SMax %138 %140
%141 = OpLoad %v4int %expectedB
%142 = OpCompositeExtract %int %141 0
%143 = OpIEqual %bool %136 %142
OpBranch %135
%135 = OpLabel
%144 = OpPhi %bool %false %129 %143 %134
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%148 = OpLoad %v4int %intValues
%149 = OpVectorShuffle %v2int %148 %148 0 1
%150 = OpLoad %v4int %intGreen
%151 = OpVectorShuffle %v2int %150 %150 0 1
%147 = OpExtInst %v2int %1 SMax %149 %151
%152 = OpLoad %v4int %expectedB
%153 = OpVectorShuffle %v2int %152 %152 0 1
%154 = OpIEqual %v2bool %147 %153
%155 = OpAll %bool %154
OpBranch %146
%146 = OpLabel
%156 = OpPhi %bool %false %135 %155 %145
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpLoad %v4int %intValues
%161 = OpVectorShuffle %v3int %160 %160 0 1 2
%162 = OpLoad %v4int %intGreen
%163 = OpVectorShuffle %v3int %162 %162 0 1 2
%159 = OpExtInst %v3int %1 SMax %161 %163
%164 = OpLoad %v4int %expectedB
%165 = OpVectorShuffle %v3int %164 %164 0 1 2
%166 = OpIEqual %v3bool %159 %165
%167 = OpAll %bool %166
OpBranch %158
%158 = OpLabel
%168 = OpPhi %bool %false %146 %167 %157
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%172 = OpLoad %v4int %intValues
%173 = OpLoad %v4int %intGreen
%171 = OpExtInst %v4int %1 SMax %172 %173
%174 = OpLoad %v4int %expectedB
%175 = OpIEqual %v4bool %171 %174
%176 = OpAll %bool %175
OpBranch %170
%170 = OpLabel
%177 = OpPhi %bool %false %158 %176 %169
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpLoad %v4int %expectedB
%181 = OpCompositeExtract %int %180 0
%182 = OpIEqual %bool %int_0 %181
OpBranch %179
%179 = OpLabel
%183 = OpPhi %bool %false %170 %182 %178
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%187 = OpLoad %v4int %expectedB
%188 = OpVectorShuffle %v2int %187 %187 0 1
%189 = OpIEqual %v2bool %186 %188
%190 = OpAll %bool %189
OpBranch %185
%185 = OpLabel
%191 = OpPhi %bool %false %179 %190 %184
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%195 = OpLoad %v4int %expectedB
%196 = OpVectorShuffle %v3int %195 %195 0 1 2
%197 = OpIEqual %v3bool %194 %196
%198 = OpAll %bool %197
OpBranch %193
%193 = OpLabel
%199 = OpPhi %bool %false %185 %198 %192
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%202 = OpLoad %v4int %expectedB
%203 = OpIEqual %v4bool %66 %202
%204 = OpAll %bool %203
OpBranch %201
%201 = OpLabel
%205 = OpPhi %bool %false %193 %204 %200
OpSelectionMerge %210 None
OpBranchConditional %205 %208 %209
%208 = OpLabel
%211 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%212 = OpLoad %v4float %211
OpStore %206 %212
OpBranch %210
%209 = OpLabel
%213 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%215 = OpLoad %v4float %213
OpStore %206 %215
OpBranch %210
%210 = OpLabel
%216 = OpLoad %v4float %206
OpReturnValue %216
OpFunctionEnd
