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
OpDecorate %180 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
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
%75 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%86 = OpConstantComposite %v3int %int_50 %int_50 %int_50
%v3bool = OpTypeVector %bool 3
%95 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%112 = OpConstantComposite %v3int %int_50 %int_50 %int_75
%157 = OpConstantComposite %v2int %int_0 %int_100
%164 = OpConstantComposite %v3int %int_0 %int_100 %int_75
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
%174 = OpVariable %_ptr_Function_v4float Function
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
%68 = OpExtInst %int %1 SMax %37 %int_50
%69 = OpIEqual %bool %68 %int_50
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpVectorShuffle %v2int %44 %44 0 1
%72 = OpExtInst %v2int %1 SMax %73 %75
%76 = OpVectorShuffle %v2int %63 %63 0 1
%78 = OpIEqual %v2bool %72 %76
%79 = OpAll %bool %78
OpBranch %71
%71 = OpLabel
%80 = OpPhi %bool %false %25 %79 %70
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpVectorShuffle %v3int %44 %44 0 1 2
%83 = OpExtInst %v3int %1 SMax %84 %86
%87 = OpVectorShuffle %v3int %63 %63 0 1 2
%89 = OpIEqual %v3bool %83 %87
%90 = OpAll %bool %89
OpBranch %82
%82 = OpLabel
%91 = OpPhi %bool %false %71 %90 %81
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpExtInst %v4int %1 SMax %44 %95
%97 = OpIEqual %v4bool %94 %63
%98 = OpAll %bool %97
OpBranch %93
%93 = OpLabel
%99 = OpPhi %bool %false %82 %98 %92
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpIEqual %bool %int_50 %int_50
OpBranch %101
%101 = OpLabel
%103 = OpPhi %bool %false %93 %102 %100
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpVectorShuffle %v2int %63 %63 0 1
%107 = OpIEqual %v2bool %75 %106
%108 = OpAll %bool %107
OpBranch %105
%105 = OpLabel
%109 = OpPhi %bool %false %101 %108 %104
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpVectorShuffle %v3int %63 %63 0 1 2
%114 = OpIEqual %v3bool %112 %113
%115 = OpAll %bool %114
OpBranch %111
%111 = OpLabel
%116 = OpPhi %bool %false %105 %115 %110
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpIEqual %v4bool %63 %63
%120 = OpAll %bool %119
OpBranch %118
%118 = OpLabel
%121 = OpPhi %bool %false %111 %120 %117
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpExtInst %int %1 SMax %37 %51
%125 = OpIEqual %bool %124 %int_0
OpBranch %123
%123 = OpLabel
%126 = OpPhi %bool %false %118 %125 %122
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%130 = OpVectorShuffle %v2int %44 %44 0 1
%131 = OpVectorShuffle %v2int %58 %58 0 1
%129 = OpExtInst %v2int %1 SMax %130 %131
%132 = OpVectorShuffle %v2int %66 %66 0 1
%133 = OpIEqual %v2bool %129 %132
%134 = OpAll %bool %133
OpBranch %128
%128 = OpLabel
%135 = OpPhi %bool %false %123 %134 %127
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%139 = OpVectorShuffle %v3int %44 %44 0 1 2
%140 = OpVectorShuffle %v3int %58 %58 0 1 2
%138 = OpExtInst %v3int %1 SMax %139 %140
%141 = OpVectorShuffle %v3int %66 %66 0 1 2
%142 = OpIEqual %v3bool %138 %141
%143 = OpAll %bool %142
OpBranch %137
%137 = OpLabel
%144 = OpPhi %bool %false %128 %143 %136
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpExtInst %v4int %1 SMax %44 %58
%148 = OpIEqual %v4bool %147 %66
%149 = OpAll %bool %148
OpBranch %146
%146 = OpLabel
%150 = OpPhi %bool %false %137 %149 %145
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpIEqual %bool %int_0 %int_0
OpBranch %152
%152 = OpLabel
%154 = OpPhi %bool %false %146 %153 %151
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpVectorShuffle %v2int %66 %66 0 1
%159 = OpIEqual %v2bool %157 %158
%160 = OpAll %bool %159
OpBranch %156
%156 = OpLabel
%161 = OpPhi %bool %false %152 %160 %155
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%165 = OpVectorShuffle %v3int %66 %66 0 1 2
%166 = OpIEqual %v3bool %164 %165
%167 = OpAll %bool %166
OpBranch %163
%163 = OpLabel
%168 = OpPhi %bool %false %156 %167 %162
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpIEqual %v4bool %66 %66
%172 = OpAll %bool %171
OpBranch %170
%170 = OpLabel
%173 = OpPhi %bool %false %163 %172 %169
OpSelectionMerge %178 None
OpBranchConditional %173 %176 %177
%176 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%180 = OpLoad %v4float %179
OpStore %174 %180
OpBranch %178
%177 = OpLabel
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%183 = OpLoad %v4float %181
OpStore %174 %183
OpBranch %178
%178 = OpLabel
%184 = OpLoad %v4float %174
OpReturnValue %184
OpFunctionEnd
