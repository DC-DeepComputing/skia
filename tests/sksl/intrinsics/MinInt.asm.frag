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
OpDecorate %179 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
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
%int_n125 = OpConstant %int -125
%int_50 = OpConstant %int 50
%62 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%int_100 = OpConstant %int 100
%65 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%74 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%85 = OpConstantComposite %v3int %int_50 %int_50 %int_50
%v3bool = OpTypeVector %bool 3
%94 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%105 = OpConstantComposite %v2int %int_n125 %int_0
%112 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
%163 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
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
OpStore %expectedA %62
OpStore %expectedB %65
%67 = OpExtInst %int %1 SMin %37 %int_50
%68 = OpIEqual %bool %67 %int_n125
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpVectorShuffle %v2int %44 %44 0 1
%71 = OpExtInst %v2int %1 SMin %72 %74
%75 = OpVectorShuffle %v2int %62 %62 0 1
%77 = OpIEqual %v2bool %71 %75
%78 = OpAll %bool %77
OpBranch %70
%70 = OpLabel
%79 = OpPhi %bool %false %25 %78 %69
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpVectorShuffle %v3int %44 %44 0 1 2
%82 = OpExtInst %v3int %1 SMin %83 %85
%86 = OpVectorShuffle %v3int %62 %62 0 1 2
%88 = OpIEqual %v3bool %82 %86
%89 = OpAll %bool %88
OpBranch %81
%81 = OpLabel
%90 = OpPhi %bool %false %70 %89 %80
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpExtInst %v4int %1 SMin %44 %94
%96 = OpIEqual %v4bool %93 %62
%97 = OpAll %bool %96
OpBranch %92
%92 = OpLabel
%98 = OpPhi %bool %false %81 %97 %91
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpIEqual %bool %int_n125 %int_n125
OpBranch %100
%100 = OpLabel
%102 = OpPhi %bool %false %92 %101 %99
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpVectorShuffle %v2int %62 %62 0 1
%107 = OpIEqual %v2bool %105 %106
%108 = OpAll %bool %107
OpBranch %104
%104 = OpLabel
%109 = OpPhi %bool %false %100 %108 %103
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpVectorShuffle %v3int %62 %62 0 1 2
%114 = OpIEqual %v3bool %112 %113
%115 = OpAll %bool %114
OpBranch %111
%111 = OpLabel
%116 = OpPhi %bool %false %104 %115 %110
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpIEqual %v4bool %62 %62
%120 = OpAll %bool %119
OpBranch %118
%118 = OpLabel
%121 = OpPhi %bool %false %111 %120 %117
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpExtInst %int %1 SMin %37 %51
%125 = OpIEqual %bool %124 %int_n125
OpBranch %123
%123 = OpLabel
%126 = OpPhi %bool %false %118 %125 %122
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%130 = OpVectorShuffle %v2int %44 %44 0 1
%131 = OpVectorShuffle %v2int %58 %58 0 1
%129 = OpExtInst %v2int %1 SMin %130 %131
%132 = OpVectorShuffle %v2int %65 %65 0 1
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
%138 = OpExtInst %v3int %1 SMin %139 %140
%141 = OpVectorShuffle %v3int %65 %65 0 1 2
%142 = OpIEqual %v3bool %138 %141
%143 = OpAll %bool %142
OpBranch %137
%137 = OpLabel
%144 = OpPhi %bool %false %128 %143 %136
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%147 = OpExtInst %v4int %1 SMin %44 %58
%148 = OpIEqual %v4bool %147 %65
%149 = OpAll %bool %148
OpBranch %146
%146 = OpLabel
%150 = OpPhi %bool %false %137 %149 %145
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%153 = OpIEqual %bool %int_n125 %int_n125
OpBranch %152
%152 = OpLabel
%154 = OpPhi %bool %false %146 %153 %151
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%157 = OpVectorShuffle %v2int %65 %65 0 1
%158 = OpIEqual %v2bool %105 %157
%159 = OpAll %bool %158
OpBranch %156
%156 = OpLabel
%160 = OpPhi %bool %false %152 %159 %155
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpVectorShuffle %v3int %65 %65 0 1 2
%165 = OpIEqual %v3bool %163 %164
%166 = OpAll %bool %165
OpBranch %162
%162 = OpLabel
%167 = OpPhi %bool %false %156 %166 %161
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpIEqual %v4bool %65 %65
%171 = OpAll %bool %170
OpBranch %169
%169 = OpLabel
%172 = OpPhi %bool %false %162 %171 %168
OpSelectionMerge %177 None
OpBranchConditional %172 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%179 = OpLoad %v4float %178
OpStore %173 %179
OpBranch %177
%176 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%182 = OpLoad %v4float %180
OpStore %173 %182
OpBranch %177
%177 = OpLabel
%183 = OpLoad %v4float %173
OpReturnValue %183
OpFunctionEnd
