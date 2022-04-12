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
OpDecorate %29 RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
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
%49 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%63 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%76 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%92 = OpConstantComposite %v2float %float_0 %float_0
%100 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%128 = OpConstantComposite %v2float %float_0 %float_1
%140 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%164 = OpConstantComposite %v2float %float_1 %float_1
%172 = OpConstantComposite %v3float %float_1 %float_1 %float_0
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
%184 = OpVariable %_ptr_Function_v4float Function
OpStore %constGreen %29
OpStore %expectedA %31
OpStore %expectedB %33
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%35 = OpExtInst %float %1 Step %float_0_5 %42
%43 = OpLoad %v4float %expectedA
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %35 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
%48 = OpExtInst %v2float %1 Step %49 %52
%53 = OpLoad %v4float %expectedA
%54 = OpVectorShuffle %v2float %53 %53 0 1
%55 = OpFOrdEqual %v2bool %48 %54
%57 = OpAll %bool %55
OpBranch %47
%47 = OpLabel
%58 = OpPhi %bool %false %25 %57 %46
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%61 = OpExtInst %v3float %1 Step %63 %66
%67 = OpLoad %v4float %expectedA
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpFOrdEqual %v3bool %61 %68
%71 = OpAll %bool %69
OpBranch %60
%60 = OpLabel
%72 = OpPhi %bool %false %47 %71 %59
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %77
%75 = OpExtInst %v4float %1 Step %76 %78
%79 = OpLoad %v4float %expectedA
%80 = OpFOrdEqual %v4bool %75 %79
%82 = OpAll %bool %80
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %60 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4float %expectedA
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdEqual %bool %float_0 %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %74 %88 %84
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%93 = OpLoad %v4float %expectedA
%94 = OpVectorShuffle %v2float %93 %93 0 1
%95 = OpFOrdEqual %v2bool %92 %94
%96 = OpAll %bool %95
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %false %85 %96 %90
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpLoad %v4float %expectedA
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpFOrdEqual %v3bool %100 %102
%104 = OpAll %bool %103
OpBranch %99
%99 = OpLabel
%105 = OpPhi %bool %false %91 %104 %98
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v4float %expectedA
%109 = OpFOrdEqual %v4bool %31 %108
%110 = OpAll %bool %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %99 %110 %106
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %115
%117 = OpCompositeExtract %float %116 0
%114 = OpExtInst %float %1 Step %117 %float_0
%118 = OpLoad %v4float %expectedB
%119 = OpCompositeExtract %float %118 0
%120 = OpFOrdEqual %bool %114 %119
OpBranch %113
%113 = OpLabel
%121 = OpPhi %bool %false %107 %120 %112
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%126 = OpLoad %v4float %125
%127 = OpVectorShuffle %v2float %126 %126 0 1
%124 = OpExtInst %v2float %1 Step %127 %128
%129 = OpLoad %v4float %expectedB
%130 = OpVectorShuffle %v2float %129 %129 0 1
%131 = OpFOrdEqual %v2bool %124 %130
%132 = OpAll %bool %131
OpBranch %123
%123 = OpLabel
%133 = OpPhi %bool %false %113 %132 %122
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%138 = OpLoad %v4float %137
%139 = OpVectorShuffle %v3float %138 %138 0 1 2
%136 = OpExtInst %v3float %1 Step %139 %140
%141 = OpLoad %v4float %expectedB
%142 = OpVectorShuffle %v3float %141 %141 0 1 2
%143 = OpFOrdEqual %v3bool %136 %142
%144 = OpAll %bool %143
OpBranch %135
%135 = OpLabel
%145 = OpPhi %bool %false %123 %144 %134
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%150 = OpLoad %v4float %149
%151 = OpLoad %v4float %constGreen
%148 = OpExtInst %v4float %1 Step %150 %151
%152 = OpLoad %v4float %expectedB
%153 = OpFOrdEqual %v4bool %148 %152
%154 = OpAll %bool %153
OpBranch %147
%147 = OpLabel
%155 = OpPhi %bool %false %135 %154 %146
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpLoad %v4float %expectedB
%159 = OpCompositeExtract %float %158 0
%160 = OpFOrdEqual %bool %float_1 %159
OpBranch %157
%157 = OpLabel
%161 = OpPhi %bool %false %147 %160 %156
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%165 = OpLoad %v4float %expectedB
%166 = OpVectorShuffle %v2float %165 %165 0 1
%167 = OpFOrdEqual %v2bool %164 %166
%168 = OpAll %bool %167
OpBranch %163
%163 = OpLabel
%169 = OpPhi %bool %false %157 %168 %162
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpLoad %v4float %expectedB
%174 = OpVectorShuffle %v3float %173 %173 0 1 2
%175 = OpFOrdEqual %v3bool %172 %174
%176 = OpAll %bool %175
OpBranch %171
%171 = OpLabel
%177 = OpPhi %bool %false %163 %176 %170
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpLoad %v4float %expectedB
%181 = OpFOrdEqual %v4bool %33 %180
%182 = OpAll %bool %181
OpBranch %179
%179 = OpLabel
%183 = OpPhi %bool %false %171 %182 %178
OpSelectionMerge %187 None
OpBranchConditional %183 %185 %186
%185 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%190 = OpLoad %v4float %188
OpStore %184 %190
OpBranch %187
%186 = OpLabel
%191 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%193 = OpLoad %v4float %191
OpStore %184 %193
OpBranch %187
%187 = OpLabel
%194 = OpLoad %v4float %184
OpReturnValue %194
OpFunctionEnd
