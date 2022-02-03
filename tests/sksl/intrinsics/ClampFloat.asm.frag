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
OpDecorate %31 RelaxedPrecision
OpDecorate %clampLow RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %clampHigh RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %190 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%float_0_00100000005 = OpConstant %float 0.00100000005
%v3float = OpTypeVector %float 3
%112 = OpConstantComposite %v2float %float_n1 %float_n2
%113 = OpConstantComposite %v2float %float_1 %float_2
%125 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
%126 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
%150 = OpConstantComposite %v2float %float_n1 %float_0
%v2bool = OpTypeVector %bool 2
%159 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%188 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
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
%200 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %clampLow %34
OpStore %expectedB %38
OpStore %clampHigh %42
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %46
%51 = OpCompositeExtract %float %50 0
%45 = OpExtInst %float %1 FClamp %51 %float_n1 %float_1
%52 = OpLoad %v4float %expectedA
%53 = OpCompositeExtract %float %52 0
%44 = OpExtInst %float %1 Distance %45 %53
%55 = OpFOrdLessThanEqual %bool %44 %float_0_00100000005
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%61 = OpLoad %v4float %60
%62 = OpVectorShuffle %v2float %61 %61 0 1
%63 = OpCompositeConstruct %v2float %float_n1 %float_n1
%64 = OpCompositeConstruct %v2float %float_1 %float_1
%59 = OpExtInst %v2float %1 FClamp %62 %63 %64
%65 = OpLoad %v4float %expectedA
%66 = OpVectorShuffle %v2float %65 %65 0 1
%58 = OpExtInst %float %1 Distance %59 %66
%67 = OpFOrdLessThanEqual %bool %58 %float_0_00100000005
OpBranch %57
%57 = OpLabel
%68 = OpPhi %bool %false %25 %67 %56
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%75 = OpVectorShuffle %v3float %74 %74 0 1 2
%77 = OpCompositeConstruct %v3float %float_n1 %float_n1 %float_n1
%78 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%72 = OpExtInst %v3float %1 FClamp %75 %77 %78
%79 = OpLoad %v4float %expectedA
%80 = OpVectorShuffle %v3float %79 %79 0 1 2
%71 = OpExtInst %float %1 Distance %72 %80
%81 = OpFOrdLessThanEqual %bool %71 %float_0_00100000005
OpBranch %70
%70 = OpLabel
%82 = OpPhi %bool %false %57 %81 %69
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%88 = OpLoad %v4float %87
%89 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%90 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%86 = OpExtInst %v4float %1 FClamp %88 %89 %90
%91 = OpLoad %v4float %expectedA
%85 = OpExtInst %float %1 Distance %86 %91
%92 = OpFOrdLessThanEqual %bool %85 %float_0_00100000005
OpBranch %84
%84 = OpLabel
%93 = OpPhi %bool %false %70 %92 %83
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpCompositeExtract %float %99 0
%97 = OpExtInst %float %1 FClamp %100 %float_n1 %float_1
%101 = OpLoad %v4float %expectedB
%102 = OpCompositeExtract %float %101 0
%96 = OpExtInst %float %1 Distance %97 %102
%103 = OpFOrdLessThanEqual %bool %96 %float_0_00100000005
OpBranch %95
%95 = OpLabel
%104 = OpPhi %bool %false %84 %103 %94
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%110 = OpLoad %v4float %109
%111 = OpVectorShuffle %v2float %110 %110 0 1
%108 = OpExtInst %v2float %1 FClamp %111 %112 %113
%114 = OpLoad %v4float %expectedB
%115 = OpVectorShuffle %v2float %114 %114 0 1
%107 = OpExtInst %float %1 Distance %108 %115
%116 = OpFOrdLessThanEqual %bool %107 %float_0_00100000005
OpBranch %106
%106 = OpLabel
%117 = OpPhi %bool %false %95 %116 %105
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%123 = OpLoad %v4float %122
%124 = OpVectorShuffle %v3float %123 %123 0 1 2
%121 = OpExtInst %v3float %1 FClamp %124 %125 %126
%127 = OpLoad %v4float %expectedB
%128 = OpVectorShuffle %v3float %127 %127 0 1 2
%120 = OpExtInst %float %1 Distance %121 %128
%129 = OpFOrdLessThanEqual %bool %120 %float_0_00100000005
OpBranch %119
%119 = OpLabel
%130 = OpPhi %bool %false %106 %129 %118
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%136 = OpLoad %v4float %135
%137 = OpLoad %v4float %clampLow
%138 = OpLoad %v4float %clampHigh
%134 = OpExtInst %v4float %1 FClamp %136 %137 %138
%139 = OpLoad %v4float %expectedB
%133 = OpExtInst %float %1 Distance %134 %139
%140 = OpFOrdLessThanEqual %bool %133 %float_0_00100000005
OpBranch %132
%132 = OpLabel
%141 = OpPhi %bool %false %119 %140 %131
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpLoad %v4float %expectedA
%145 = OpCompositeExtract %float %144 0
%146 = OpFOrdEqual %bool %float_n1 %145
OpBranch %143
%143 = OpLabel
%147 = OpPhi %bool %false %132 %146 %142
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpLoad %v4float %expectedA
%152 = OpVectorShuffle %v2float %151 %151 0 1
%153 = OpFOrdEqual %v2bool %150 %152
%155 = OpAll %bool %153
OpBranch %149
%149 = OpLabel
%156 = OpPhi %bool %false %143 %155 %148
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpLoad %v4float %expectedA
%161 = OpVectorShuffle %v3float %160 %160 0 1 2
%162 = OpFOrdEqual %v3bool %159 %161
%164 = OpAll %bool %162
OpBranch %158
%158 = OpLabel
%165 = OpPhi %bool %false %149 %164 %157
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%168 = OpLoad %v4float %expectedA
%169 = OpFOrdEqual %v4bool %31 %168
%171 = OpAll %bool %169
OpBranch %167
%167 = OpLabel
%172 = OpPhi %bool %false %158 %171 %166
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpLoad %v4float %expectedB
%176 = OpCompositeExtract %float %175 0
%177 = OpFOrdEqual %bool %float_n1 %176
OpBranch %174
%174 = OpLabel
%178 = OpPhi %bool %false %167 %177 %173
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpLoad %v4float %expectedB
%182 = OpVectorShuffle %v2float %181 %181 0 1
%183 = OpFOrdEqual %v2bool %150 %182
%184 = OpAll %bool %183
OpBranch %180
%180 = OpLabel
%185 = OpPhi %bool %false %174 %184 %179
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%189 = OpLoad %v4float %expectedB
%190 = OpVectorShuffle %v3float %189 %189 0 1 2
%191 = OpFOrdEqual %v3bool %188 %190
%192 = OpAll %bool %191
OpBranch %187
%187 = OpLabel
%193 = OpPhi %bool %false %180 %192 %186
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%196 = OpLoad %v4float %expectedB
%197 = OpFOrdEqual %v4bool %38 %196
%198 = OpAll %bool %197
OpBranch %195
%195 = OpLabel
%199 = OpPhi %bool %false %187 %198 %194
OpSelectionMerge %203 None
OpBranchConditional %199 %201 %202
%201 = OpLabel
%204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%206 = OpLoad %v4float %204
OpStore %200 %206
OpBranch %203
%202 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%209 = OpLoad %v4float %207
OpStore %200 %209
OpBranch %203
%203 = OpLabel
%210 = OpLoad %v4float %200
OpReturnValue %210
OpFunctionEnd
