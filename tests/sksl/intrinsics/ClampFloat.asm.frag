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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %clampLow RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %clampHigh RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%31 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%float_n2 = OpConstant %float -2
%34 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_0_5 = OpConstant %float 0.5
%float_2_25 = OpConstant %float 2.25
%38 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%42 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%58 = OpConstantComposite %v2float %float_n1 %float_n1
%59 = OpConstantComposite %v2float %float_1 %float_1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%73 = OpConstantComposite %v3float %float_n1 %float_n1 %float_n1
%74 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%v3bool = OpTypeVector %bool 3
%86 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
%87 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%109 = OpConstantComposite %v2float %float_n1 %float_n2
%110 = OpConstantComposite %v2float %float_1 %float_2
%122 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
%123 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
%148 = OpConstantComposite %v2float %float_n1 %float_0
%156 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
%183 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
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
%expectedA = OpVariable %_ptr_Function_v4float Function
%clampLow = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%clampHigh = OpVariable %_ptr_Function_v4float Function
%195 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %clampLow %34
OpStore %expectedB %38
OpStore %clampHigh %42
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %45
%50 = OpCompositeExtract %float %49 0
%44 = OpExtInst %float %1 FClamp %50 %float_n1 %float_1
%51 = OpFOrdEqual %bool %44 %float_n1
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%56 = OpLoad %v4float %55
%57 = OpVectorShuffle %v2float %56 %56 0 1
%54 = OpExtInst %v2float %1 FClamp %57 %58 %59
%60 = OpLoad %v4float %expectedA
%61 = OpVectorShuffle %v2float %60 %60 0 1
%62 = OpFOrdEqual %v2bool %54 %61
%64 = OpAll %bool %62
OpBranch %53
%53 = OpLabel
%65 = OpPhi %bool %false %25 %64 %52
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v3float %70 %70 0 1 2
%68 = OpExtInst %v3float %1 FClamp %71 %73 %74
%75 = OpLoad %v4float %expectedA
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%77 = OpFOrdEqual %v3bool %68 %76
%79 = OpAll %bool %77
OpBranch %67
%67 = OpLabel
%80 = OpPhi %bool %false %53 %79 %66
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %84
%83 = OpExtInst %v4float %1 FClamp %85 %86 %87
%88 = OpLoad %v4float %expectedA
%89 = OpFOrdEqual %v4bool %83 %88
%91 = OpAll %bool %89
OpBranch %82
%82 = OpLabel
%92 = OpPhi %bool %false %67 %91 %81
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%97 = OpLoad %v4float %96
%98 = OpCompositeExtract %float %97 0
%95 = OpExtInst %float %1 FClamp %98 %float_n1 %float_1
%99 = OpLoad %v4float %expectedB
%100 = OpCompositeExtract %float %99 0
%101 = OpFOrdEqual %bool %95 %100
OpBranch %94
%94 = OpLabel
%102 = OpPhi %bool %false %82 %101 %93
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v2float %107 %107 0 1
%105 = OpExtInst %v2float %1 FClamp %108 %109 %110
%111 = OpLoad %v4float %expectedB
%112 = OpVectorShuffle %v2float %111 %111 0 1
%113 = OpFOrdEqual %v2bool %105 %112
%114 = OpAll %bool %113
OpBranch %104
%104 = OpLabel
%115 = OpPhi %bool %false %94 %114 %103
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%120 = OpLoad %v4float %119
%121 = OpVectorShuffle %v3float %120 %120 0 1 2
%118 = OpExtInst %v3float %1 FClamp %121 %122 %123
%124 = OpLoad %v4float %expectedB
%125 = OpVectorShuffle %v3float %124 %124 0 1 2
%126 = OpFOrdEqual %v3bool %118 %125
%127 = OpAll %bool %126
OpBranch %117
%117 = OpLabel
%128 = OpPhi %bool %false %104 %127 %116
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%133 = OpLoad %v4float %132
%134 = OpLoad %v4float %clampLow
%135 = OpLoad %v4float %clampHigh
%131 = OpExtInst %v4float %1 FClamp %133 %134 %135
%136 = OpLoad %v4float %expectedB
%137 = OpFOrdEqual %v4bool %131 %136
%138 = OpAll %bool %137
OpBranch %130
%130 = OpLabel
%139 = OpPhi %bool %false %117 %138 %129
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%142 = OpLoad %v4float %expectedA
%143 = OpCompositeExtract %float %142 0
%144 = OpFOrdEqual %bool %float_n1 %143
OpBranch %141
%141 = OpLabel
%145 = OpPhi %bool %false %130 %144 %140
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%149 = OpLoad %v4float %expectedA
%150 = OpVectorShuffle %v2float %149 %149 0 1
%151 = OpFOrdEqual %v2bool %148 %150
%152 = OpAll %bool %151
OpBranch %147
%147 = OpLabel
%153 = OpPhi %bool %false %141 %152 %146
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%157 = OpLoad %v4float %expectedA
%158 = OpVectorShuffle %v3float %157 %157 0 1 2
%159 = OpFOrdEqual %v3bool %156 %158
%160 = OpAll %bool %159
OpBranch %155
%155 = OpLabel
%161 = OpPhi %bool %false %147 %160 %154
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %v4float %expectedA
%165 = OpFOrdEqual %v4bool %31 %164
%166 = OpAll %bool %165
OpBranch %163
%163 = OpLabel
%167 = OpPhi %bool %false %155 %166 %162
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%170 = OpLoad %v4float %expectedB
%171 = OpCompositeExtract %float %170 0
%172 = OpFOrdEqual %bool %float_n1 %171
OpBranch %169
%169 = OpLabel
%173 = OpPhi %bool %false %163 %172 %168
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%176 = OpLoad %v4float %expectedB
%177 = OpVectorShuffle %v2float %176 %176 0 1
%178 = OpFOrdEqual %v2bool %148 %177
%179 = OpAll %bool %178
OpBranch %175
%175 = OpLabel
%180 = OpPhi %bool %false %169 %179 %174
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%184 = OpLoad %v4float %expectedB
%185 = OpVectorShuffle %v3float %184 %184 0 1 2
%186 = OpFOrdEqual %v3bool %183 %185
%187 = OpAll %bool %186
OpBranch %182
%182 = OpLabel
%188 = OpPhi %bool %false %175 %187 %181
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %v4float %expectedB
%192 = OpFOrdEqual %v4bool %38 %191
%193 = OpAll %bool %192
OpBranch %190
%190 = OpLabel
%194 = OpPhi %bool %false %182 %193 %189
OpSelectionMerge %198 None
OpBranchConditional %194 %196 %197
%196 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%201 = OpLoad %v4float %199
OpStore %195 %201
OpBranch %198
%197 = OpLabel
%202 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%204 = OpLoad %v4float %202
OpStore %195 %204
OpBranch %198
%198 = OpLabel
%205 = OpLoad %v4float %195
OpReturnValue %205
OpFunctionEnd
