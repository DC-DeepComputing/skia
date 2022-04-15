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
OpName %constGreen "constGreen"
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
OpDecorate %constGreen RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%31 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
%33 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_0
%false = OpConstantFalse %bool
%float_0_5 = OpConstant %float 0.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%47 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%61 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%74 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%97 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%125 = OpConstantComposite %v2float %float_0 %float_1
%137 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%161 = OpConstantComposite %v2float %float_1 %float_1
%169 = OpConstantComposite %v3float %float_1 %float_1 %float_0
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
%constGreen = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%181 = OpVariable %_ptr_Function_v4float Function
OpStore %constGreen %29
OpStore %expectedA %31
OpStore %expectedB %33
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%35 = OpExtInst %float %1 Step %float_0_5 %42
%43 = OpFOrdEqual %bool %35 %float_0
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%46 = OpExtInst %v2float %1 Step %47 %50
%51 = OpLoad %v4float %expectedA
%52 = OpVectorShuffle %v2float %51 %51 0 1
%53 = OpFOrdEqual %v2bool %46 %52
%55 = OpAll %bool %53
OpBranch %45
%45 = OpLabel
%56 = OpPhi %bool %false %25 %55 %44
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%59 = OpExtInst %v3float %1 Step %61 %64
%65 = OpLoad %v4float %expectedA
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%67 = OpFOrdEqual %v3bool %59 %66
%69 = OpAll %bool %67
OpBranch %58
%58 = OpLabel
%70 = OpPhi %bool %false %45 %69 %57
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%76 = OpLoad %v4float %75
%73 = OpExtInst %v4float %1 Step %74 %76
%77 = OpLoad %v4float %expectedA
%78 = OpFOrdEqual %v4bool %73 %77
%80 = OpAll %bool %78
OpBranch %72
%72 = OpLabel
%81 = OpPhi %bool %false %58 %80 %71
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpLoad %v4float %expectedA
%85 = OpCompositeExtract %float %84 0
%86 = OpFOrdEqual %bool %float_0 %85
OpBranch %83
%83 = OpLabel
%87 = OpPhi %bool %false %72 %86 %82
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %v4float %expectedA
%91 = OpVectorShuffle %v2float %90 %90 0 1
%92 = OpFOrdEqual %v2bool %19 %91
%93 = OpAll %bool %92
OpBranch %89
%89 = OpLabel
%94 = OpPhi %bool %false %83 %93 %88
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpLoad %v4float %expectedA
%99 = OpVectorShuffle %v3float %98 %98 0 1 2
%100 = OpFOrdEqual %v3bool %97 %99
%101 = OpAll %bool %100
OpBranch %96
%96 = OpLabel
%102 = OpPhi %bool %false %89 %101 %95
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %v4float %expectedA
%106 = OpFOrdEqual %v4bool %31 %105
%107 = OpAll %bool %106
OpBranch %104
%104 = OpLabel
%108 = OpPhi %bool %false %96 %107 %103
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%113 = OpLoad %v4float %112
%114 = OpCompositeExtract %float %113 0
%111 = OpExtInst %float %1 Step %114 %float_0
%115 = OpLoad %v4float %expectedB
%116 = OpCompositeExtract %float %115 0
%117 = OpFOrdEqual %bool %111 %116
OpBranch %110
%110 = OpLabel
%118 = OpPhi %bool %false %104 %117 %109
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%123 = OpLoad %v4float %122
%124 = OpVectorShuffle %v2float %123 %123 0 1
%121 = OpExtInst %v2float %1 Step %124 %125
%126 = OpLoad %v4float %expectedB
%127 = OpVectorShuffle %v2float %126 %126 0 1
%128 = OpFOrdEqual %v2bool %121 %127
%129 = OpAll %bool %128
OpBranch %120
%120 = OpLabel
%130 = OpPhi %bool %false %110 %129 %119
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%135 = OpLoad %v4float %134
%136 = OpVectorShuffle %v3float %135 %135 0 1 2
%133 = OpExtInst %v3float %1 Step %136 %137
%138 = OpLoad %v4float %expectedB
%139 = OpVectorShuffle %v3float %138 %138 0 1 2
%140 = OpFOrdEqual %v3bool %133 %139
%141 = OpAll %bool %140
OpBranch %132
%132 = OpLabel
%142 = OpPhi %bool %false %120 %141 %131
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%147 = OpLoad %v4float %146
%148 = OpLoad %v4float %constGreen
%145 = OpExtInst %v4float %1 Step %147 %148
%149 = OpLoad %v4float %expectedB
%150 = OpFOrdEqual %v4bool %145 %149
%151 = OpAll %bool %150
OpBranch %144
%144 = OpLabel
%152 = OpPhi %bool %false %132 %151 %143
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpLoad %v4float %expectedB
%156 = OpCompositeExtract %float %155 0
%157 = OpFOrdEqual %bool %float_1 %156
OpBranch %154
%154 = OpLabel
%158 = OpPhi %bool %false %144 %157 %153
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%162 = OpLoad %v4float %expectedB
%163 = OpVectorShuffle %v2float %162 %162 0 1
%164 = OpFOrdEqual %v2bool %161 %163
%165 = OpAll %bool %164
OpBranch %160
%160 = OpLabel
%166 = OpPhi %bool %false %154 %165 %159
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%170 = OpLoad %v4float %expectedB
%171 = OpVectorShuffle %v3float %170 %170 0 1 2
%172 = OpFOrdEqual %v3bool %169 %171
%173 = OpAll %bool %172
OpBranch %168
%168 = OpLabel
%174 = OpPhi %bool %false %160 %173 %167
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %v4float %expectedB
%178 = OpFOrdEqual %v4bool %33 %177
%179 = OpAll %bool %178
OpBranch %176
%176 = OpLabel
%180 = OpPhi %bool %false %168 %179 %175
OpSelectionMerge %184 None
OpBranchConditional %180 %182 %183
%182 = OpLabel
%185 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%187 = OpLoad %v4float %185
OpStore %181 %187
OpBranch %184
%183 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%190 = OpLoad %v4float %188
OpStore %181 %190
OpBranch %184
%184 = OpLabel
%191 = OpLoad %v4float %181
OpReturnValue %191
OpFunctionEnd
