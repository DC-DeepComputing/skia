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
OpName %expectedA "expectedA"
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
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
%int_n100 = OpConstant %int -100
%int_75 = OpConstant %int 75
%int_100 = OpConstant %int 100
%49 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%int_n200 = OpConstant %int -200
%52 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_50 = OpConstant %int 50
%int_225 = OpConstant %int 225
%56 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
%int_200 = OpConstant %int 200
%int_300 = OpConstant %int 300
%60 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%69 = OpConstantComposite %v2int %int_n100 %int_n100
%70 = OpConstantComposite %v2int %int_100 %int_100
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%81 = OpConstantComposite %v3int %int_n100 %int_n100 %int_n100
%82 = OpConstantComposite %v3int %int_100 %int_100 %int_100
%v3bool = OpTypeVector %bool 3
%91 = OpConstantComposite %v4int %int_n100 %int_n100 %int_n100 %int_n100
%92 = OpConstantComposite %v4int %int_100 %int_100 %int_100 %int_100
%v4bool = OpTypeVector %bool 4
%103 = OpConstantComposite %v2int %int_n100 %int_0
%110 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
%129 = OpConstantComposite %v2int %int_n100 %int_n200
%130 = OpConstantComposite %v2int %int_100 %int_200
%139 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
%140 = OpConstantComposite %v3int %int_100 %int_200 %int_50
%163 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
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
%expectedA = OpVariable %_ptr_Function_v4int Function
%clampLow = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%clampHigh = OpVariable %_ptr_Function_v4int Function
%173 = OpVariable %_ptr_Function_v4float Function
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
OpStore %expectedA %49
OpStore %clampLow %52
OpStore %expectedB %56
OpStore %clampHigh %60
%62 = OpExtInst %int %1 SClamp %37 %int_n100 %int_100
%63 = OpIEqual %bool %62 %int_n100
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpVectorShuffle %v2int %44 %44 0 1
%66 = OpExtInst %v2int %1 SClamp %67 %69 %70
%71 = OpVectorShuffle %v2int %49 %49 0 1
%73 = OpIEqual %v2bool %66 %71
%74 = OpAll %bool %73
OpBranch %65
%65 = OpLabel
%75 = OpPhi %bool %false %25 %74 %64
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpVectorShuffle %v3int %44 %44 0 1 2
%78 = OpExtInst %v3int %1 SClamp %79 %81 %82
%83 = OpVectorShuffle %v3int %49 %49 0 1 2
%85 = OpIEqual %v3bool %78 %83
%86 = OpAll %bool %85
OpBranch %77
%77 = OpLabel
%87 = OpPhi %bool %false %65 %86 %76
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpExtInst %v4int %1 SClamp %44 %91 %92
%94 = OpIEqual %v4bool %90 %49
%95 = OpAll %bool %94
OpBranch %89
%89 = OpLabel
%96 = OpPhi %bool %false %77 %95 %88
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpIEqual %bool %int_n100 %int_n100
OpBranch %98
%98 = OpLabel
%100 = OpPhi %bool %false %89 %99 %97
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpVectorShuffle %v2int %49 %49 0 1
%105 = OpIEqual %v2bool %103 %104
%106 = OpAll %bool %105
OpBranch %102
%102 = OpLabel
%107 = OpPhi %bool %false %98 %106 %101
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpVectorShuffle %v3int %49 %49 0 1 2
%112 = OpIEqual %v3bool %110 %111
%113 = OpAll %bool %112
OpBranch %109
%109 = OpLabel
%114 = OpPhi %bool %false %102 %113 %108
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%117 = OpIEqual %v4bool %49 %49
%118 = OpAll %bool %117
OpBranch %116
%116 = OpLabel
%119 = OpPhi %bool %false %109 %118 %115
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpExtInst %int %1 SClamp %37 %int_n100 %int_100
%123 = OpIEqual %bool %122 %int_n100
OpBranch %121
%121 = OpLabel
%124 = OpPhi %bool %false %116 %123 %120
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpVectorShuffle %v2int %44 %44 0 1
%127 = OpExtInst %v2int %1 SClamp %128 %129 %130
%131 = OpVectorShuffle %v2int %56 %56 0 1
%132 = OpIEqual %v2bool %127 %131
%133 = OpAll %bool %132
OpBranch %126
%126 = OpLabel
%134 = OpPhi %bool %false %121 %133 %125
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpVectorShuffle %v3int %44 %44 0 1 2
%137 = OpExtInst %v3int %1 SClamp %138 %139 %140
%141 = OpVectorShuffle %v3int %56 %56 0 1 2
%142 = OpIEqual %v3bool %137 %141
%143 = OpAll %bool %142
OpBranch %136
%136 = OpLabel
%144 = OpPhi %bool %false %126 %143 %135
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpExtInst %v4int %1 SClamp %44 %52 %60
%148 = OpIEqual %v4bool %147 %56
%149 = OpAll %bool %148
OpBranch %146
%146 = OpLabel
%150 = OpPhi %bool %false %136 %149 %145
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpIEqual %bool %int_n100 %int_n100
OpBranch %152
%152 = OpLabel
%154 = OpPhi %bool %false %146 %153 %151
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpVectorShuffle %v2int %56 %56 0 1
%158 = OpIEqual %v2bool %103 %157
%159 = OpAll %bool %158
OpBranch %156
%156 = OpLabel
%160 = OpPhi %bool %false %152 %159 %155
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpVectorShuffle %v3int %56 %56 0 1 2
%165 = OpIEqual %v3bool %163 %164
%166 = OpAll %bool %165
OpBranch %162
%162 = OpLabel
%167 = OpPhi %bool %false %156 %166 %161
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpIEqual %v4bool %56 %56
%171 = OpAll %bool %170
OpBranch %169
%169 = OpLabel
%172 = OpPhi %bool %false %162 %171 %168
OpSelectionMerge %177 None
OpBranchConditional %172 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%180 = OpLoad %v4float %178
OpStore %173 %180
OpBranch %177
%176 = OpLabel
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%183 = OpLoad %v4float %181
OpStore %173 %183
OpBranch %177
%177 = OpLabel
%184 = OpLoad %v4float %173
OpReturnValue %184
OpFunctionEnd
