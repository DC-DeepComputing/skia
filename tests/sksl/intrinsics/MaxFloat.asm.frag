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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
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
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_1 = OpConstant %float 1
%34 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%50 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%64 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%76 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%v4bool = OpTypeVector %bool 4
%97 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
%int_1 = OpConstant %int 1
%170 = OpConstantComposite %v2float %float_0 %float_1
%178 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
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
%expectedB = OpVariable %_ptr_Function_v4float Function
%190 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %expectedB %34
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%36 = OpExtInst %float %1 FMax %42 %float_0_5
%43 = OpFOrdEqual %bool %36 %float_0_5
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%48 = OpLoad %v4float %47
%49 = OpVectorShuffle %v2float %48 %48 0 1
%46 = OpExtInst %v2float %1 FMax %49 %50
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
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%61 = OpLoad %v4float %60
%62 = OpVectorShuffle %v3float %61 %61 0 1 2
%59 = OpExtInst %v3float %1 FMax %62 %64
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
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%75 = OpLoad %v4float %74
%73 = OpExtInst %v4float %1 FMax %75 %76
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
%86 = OpFOrdEqual %bool %float_0_5 %85
OpBranch %83
%83 = OpLabel
%87 = OpPhi %bool %false %72 %86 %82
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %v4float %expectedA
%91 = OpVectorShuffle %v2float %90 %90 0 1
%92 = OpFOrdEqual %v2bool %50 %91
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
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%117 = OpLoad %v4float %115
%118 = OpCompositeExtract %float %117 0
%111 = OpExtInst %float %1 FMax %114 %118
%119 = OpLoad %v4float %expectedB
%120 = OpCompositeExtract %float %119 0
%121 = OpFOrdEqual %bool %111 %120
OpBranch %110
%110 = OpLabel
%122 = OpPhi %bool %false %104 %121 %109
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%127 = OpLoad %v4float %126
%128 = OpVectorShuffle %v2float %127 %127 0 1
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%130 = OpLoad %v4float %129
%131 = OpVectorShuffle %v2float %130 %130 0 1
%125 = OpExtInst %v2float %1 FMax %128 %131
%132 = OpLoad %v4float %expectedB
%133 = OpVectorShuffle %v2float %132 %132 0 1
%134 = OpFOrdEqual %v2bool %125 %133
%135 = OpAll %bool %134
OpBranch %124
%124 = OpLabel
%136 = OpPhi %bool %false %110 %135 %123
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%141 = OpLoad %v4float %140
%142 = OpVectorShuffle %v3float %141 %141 0 1 2
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%144 = OpLoad %v4float %143
%145 = OpVectorShuffle %v3float %144 %144 0 1 2
%139 = OpExtInst %v3float %1 FMax %142 %145
%146 = OpLoad %v4float %expectedB
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%148 = OpFOrdEqual %v3bool %139 %147
%149 = OpAll %bool %148
OpBranch %138
%138 = OpLabel
%150 = OpPhi %bool %false %124 %149 %137
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%155 = OpLoad %v4float %154
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%157 = OpLoad %v4float %156
%153 = OpExtInst %v4float %1 FMax %155 %157
%158 = OpLoad %v4float %expectedB
%159 = OpFOrdEqual %v4bool %153 %158
%160 = OpAll %bool %159
OpBranch %152
%152 = OpLabel
%161 = OpPhi %bool %false %138 %160 %151
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpLoad %v4float %expectedB
%165 = OpCompositeExtract %float %164 0
%166 = OpFOrdEqual %bool %float_0 %165
OpBranch %163
%163 = OpLabel
%167 = OpPhi %bool %false %152 %166 %162
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%171 = OpLoad %v4float %expectedB
%172 = OpVectorShuffle %v2float %171 %171 0 1
%173 = OpFOrdEqual %v2bool %170 %172
%174 = OpAll %bool %173
OpBranch %169
%169 = OpLabel
%175 = OpPhi %bool %false %163 %174 %168
OpSelectionMerge %177 None
OpBranchConditional %175 %176 %177
%176 = OpLabel
%179 = OpLoad %v4float %expectedB
%180 = OpVectorShuffle %v3float %179 %179 0 1 2
%181 = OpFOrdEqual %v3bool %178 %180
%182 = OpAll %bool %181
OpBranch %177
%177 = OpLabel
%183 = OpPhi %bool %false %169 %182 %176
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %v4float %expectedB
%187 = OpFOrdEqual %v4bool %34 %186
%188 = OpAll %bool %187
OpBranch %185
%185 = OpLabel
%189 = OpPhi %bool %false %177 %188 %184
OpSelectionMerge %193 None
OpBranchConditional %189 %191 %192
%191 = OpLabel
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%195 = OpLoad %v4float %194
OpStore %190 %195
OpBranch %193
%192 = OpLabel
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%198 = OpLoad %v4float %196
OpStore %190 %198
OpBranch %193
%193 = OpLabel
%199 = OpLoad %v4float %190
OpReturnValue %199
OpFunctionEnd
