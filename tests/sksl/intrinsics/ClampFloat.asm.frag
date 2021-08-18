OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
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
OpDecorate %sk_FragCoord BuiltIn FragCoord
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %expectedA RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %clampLow RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %clampHigh RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%float_n2 = OpConstant %float -2
%36 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_0_5 = OpConstant %float 0.5
%float_2_25 = OpConstant %float 2.25
%40 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%44 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%105 = OpConstantComposite %v2float %float_n1 %float_0
%113 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
%141 = OpConstantComposite %v2float %float_n1 %float_n2
%142 = OpConstantComposite %v2float %float_1 %float_2
%154 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
%155 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
%187 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%expectedA = OpVariable %_ptr_Function_v4float Function
%clampLow = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%clampHigh = OpVariable %_ptr_Function_v4float Function
%199 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %33
OpStore %clampLow %36
OpStore %expectedB %40
OpStore %clampHigh %44
%47 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%51 = OpLoad %v4float %47
%52 = OpCompositeExtract %float %51 0
%46 = OpExtInst %float %1 FClamp %52 %float_n1 %float_1
%53 = OpLoad %v4float %expectedA
%54 = OpCompositeExtract %float %53 0
%55 = OpFOrdEqual %bool %46 %54
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v2float %60 %60 0 1
%62 = OpCompositeConstruct %v2float %float_n1 %float_n1
%63 = OpCompositeConstruct %v2float %float_1 %float_1
%58 = OpExtInst %v2float %1 FClamp %61 %62 %63
%64 = OpLoad %v4float %expectedA
%65 = OpVectorShuffle %v2float %64 %64 0 1
%66 = OpFOrdEqual %v2bool %58 %65
%68 = OpAll %bool %66
OpBranch %57
%57 = OpLabel
%69 = OpPhi %bool %false %27 %68 %56
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%74 = OpLoad %v4float %73
%75 = OpVectorShuffle %v3float %74 %74 0 1 2
%77 = OpCompositeConstruct %v3float %float_n1 %float_n1 %float_n1
%78 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%72 = OpExtInst %v3float %1 FClamp %75 %77 %78
%79 = OpLoad %v4float %expectedA
%80 = OpVectorShuffle %v3float %79 %79 0 1 2
%81 = OpFOrdEqual %v3bool %72 %80
%83 = OpAll %bool %81
OpBranch %71
%71 = OpLabel
%84 = OpPhi %bool %false %57 %83 %70
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%89 = OpLoad %v4float %88
%90 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%91 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%87 = OpExtInst %v4float %1 FClamp %89 %90 %91
%92 = OpLoad %v4float %expectedA
%93 = OpFOrdEqual %v4bool %87 %92
%95 = OpAll %bool %93
OpBranch %86
%86 = OpLabel
%96 = OpPhi %bool %false %71 %95 %85
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %v4float %expectedA
%100 = OpCompositeExtract %float %99 0
%101 = OpFOrdEqual %bool %float_n1 %100
OpBranch %98
%98 = OpLabel
%102 = OpPhi %bool %false %86 %101 %97
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v4float %expectedA
%107 = OpVectorShuffle %v2float %106 %106 0 1
%108 = OpFOrdEqual %v2bool %105 %107
%109 = OpAll %bool %108
OpBranch %104
%104 = OpLabel
%110 = OpPhi %bool %false %98 %109 %103
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpLoad %v4float %expectedA
%115 = OpVectorShuffle %v3float %114 %114 0 1 2
%116 = OpFOrdEqual %v3bool %113 %115
%117 = OpAll %bool %116
OpBranch %112
%112 = OpLabel
%118 = OpPhi %bool %false %104 %117 %111
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpLoad %v4float %expectedA
%122 = OpFOrdEqual %v4bool %33 %121
%123 = OpAll %bool %122
OpBranch %120
%120 = OpLabel
%124 = OpPhi %bool %false %112 %123 %119
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 0
%127 = OpExtInst %float %1 FClamp %130 %float_n1 %float_1
%131 = OpLoad %v4float %expectedB
%132 = OpCompositeExtract %float %131 0
%133 = OpFOrdEqual %bool %127 %132
OpBranch %126
%126 = OpLabel
%134 = OpPhi %bool %false %120 %133 %125
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%139 = OpLoad %v4float %138
%140 = OpVectorShuffle %v2float %139 %139 0 1
%137 = OpExtInst %v2float %1 FClamp %140 %141 %142
%143 = OpLoad %v4float %expectedB
%144 = OpVectorShuffle %v2float %143 %143 0 1
%145 = OpFOrdEqual %v2bool %137 %144
%146 = OpAll %bool %145
OpBranch %136
%136 = OpLabel
%147 = OpPhi %bool %false %126 %146 %135
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%152 = OpLoad %v4float %151
%153 = OpVectorShuffle %v3float %152 %152 0 1 2
%150 = OpExtInst %v3float %1 FClamp %153 %154 %155
%156 = OpLoad %v4float %expectedB
%157 = OpVectorShuffle %v3float %156 %156 0 1 2
%158 = OpFOrdEqual %v3bool %150 %157
%159 = OpAll %bool %158
OpBranch %149
%149 = OpLabel
%160 = OpPhi %bool %false %136 %159 %148
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%165 = OpLoad %v4float %164
%166 = OpLoad %v4float %clampLow
%167 = OpLoad %v4float %clampHigh
%163 = OpExtInst %v4float %1 FClamp %165 %166 %167
%168 = OpLoad %v4float %expectedB
%169 = OpFOrdEqual %v4bool %163 %168
%170 = OpAll %bool %169
OpBranch %162
%162 = OpLabel
%171 = OpPhi %bool %false %149 %170 %161
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%174 = OpLoad %v4float %expectedB
%175 = OpCompositeExtract %float %174 0
%176 = OpFOrdEqual %bool %float_n1 %175
OpBranch %173
%173 = OpLabel
%177 = OpPhi %bool %false %162 %176 %172
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpLoad %v4float %expectedB
%181 = OpVectorShuffle %v2float %180 %180 0 1
%182 = OpFOrdEqual %v2bool %105 %181
%183 = OpAll %bool %182
OpBranch %179
%179 = OpLabel
%184 = OpPhi %bool %false %173 %183 %178
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%188 = OpLoad %v4float %expectedB
%189 = OpVectorShuffle %v3float %188 %188 0 1 2
%190 = OpFOrdEqual %v3bool %187 %189
%191 = OpAll %bool %190
OpBranch %186
%186 = OpLabel
%192 = OpPhi %bool %false %179 %191 %185
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpLoad %v4float %expectedB
%196 = OpFOrdEqual %v4bool %40 %195
%197 = OpAll %bool %196
OpBranch %194
%194 = OpLabel
%198 = OpPhi %bool %false %186 %197 %193
OpSelectionMerge %202 None
OpBranchConditional %198 %200 %201
%200 = OpLabel
%203 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%205 = OpLoad %v4float %203
OpStore %199 %205
OpBranch %202
%201 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%208 = OpLoad %v4float %206
OpStore %199 %208
OpBranch %202
%202 = OpLabel
%209 = OpLoad %v4float %199
OpReturnValue %209
OpFunctionEnd
