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
OpName %expectedB "expectedB"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
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
%float_n1_25 = OpConstant %float -1.25
%float_0_5 = OpConstant %float 0.5
%32 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
%float_1 = OpConstant %float 1
%35 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%93 = OpConstantComposite %v2float %float_n1_25 %float_0
%101 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_5
%int_1 = OpConstant %int 1
%181 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0
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
%expectedB = OpVariable %_ptr_Function_v4float Function
%193 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %32
OpStore %expectedB %35
%38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%42 = OpLoad %v4float %38
%43 = OpCompositeExtract %float %42 0
%37 = OpExtInst %float %1 FMin %43 %float_0_5
%44 = OpLoad %v4float %expectedA
%45 = OpCompositeExtract %float %44 0
%46 = OpFOrdEqual %bool %37 %45
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
%53 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%49 = OpExtInst %v2float %1 FMin %52 %53
%54 = OpLoad %v4float %expectedA
%55 = OpVectorShuffle %v2float %54 %54 0 1
%56 = OpFOrdEqual %v2bool %49 %55
%58 = OpAll %bool %56
OpBranch %48
%48 = OpLabel
%59 = OpPhi %bool %false %27 %58 %47
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%67 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%62 = OpExtInst %v3float %1 FMin %65 %67
%68 = OpLoad %v4float %expectedA
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%70 = OpFOrdEqual %v3bool %62 %69
%72 = OpAll %bool %70
OpBranch %61
%61 = OpLabel
%73 = OpPhi %bool %false %48 %72 %60
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%78 = OpLoad %v4float %77
%79 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%76 = OpExtInst %v4float %1 FMin %78 %79
%80 = OpLoad %v4float %expectedA
%81 = OpFOrdEqual %v4bool %76 %80
%83 = OpAll %bool %81
OpBranch %75
%75 = OpLabel
%84 = OpPhi %bool %false %61 %83 %74
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %v4float %expectedA
%88 = OpCompositeExtract %float %87 0
%89 = OpFOrdEqual %bool %float_n1_25 %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %75 %89 %85
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpLoad %v4float %expectedA
%95 = OpVectorShuffle %v2float %94 %94 0 1
%96 = OpFOrdEqual %v2bool %93 %95
%97 = OpAll %bool %96
OpBranch %92
%92 = OpLabel
%98 = OpPhi %bool %false %86 %97 %91
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpLoad %v4float %expectedA
%103 = OpVectorShuffle %v3float %102 %102 0 1 2
%104 = OpFOrdEqual %v3bool %101 %103
%105 = OpAll %bool %104
OpBranch %100
%100 = OpLabel
%106 = OpPhi %bool %false %92 %105 %99
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4float %expectedA
%110 = OpFOrdEqual %v4bool %32 %109
%111 = OpAll %bool %110
OpBranch %108
%108 = OpLabel
%112 = OpPhi %bool %false %100 %111 %107
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%117 = OpLoad %v4float %116
%118 = OpCompositeExtract %float %117 0
%119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%121 = OpLoad %v4float %119
%122 = OpCompositeExtract %float %121 0
%115 = OpExtInst %float %1 FMin %118 %122
%123 = OpLoad %v4float %expectedB
%124 = OpCompositeExtract %float %123 0
%125 = OpFOrdEqual %bool %115 %124
OpBranch %114
%114 = OpLabel
%126 = OpPhi %bool %false %108 %125 %113
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%131 = OpLoad %v4float %130
%132 = OpVectorShuffle %v2float %131 %131 0 1
%133 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%134 = OpLoad %v4float %133
%135 = OpVectorShuffle %v2float %134 %134 0 1
%129 = OpExtInst %v2float %1 FMin %132 %135
%136 = OpLoad %v4float %expectedB
%137 = OpVectorShuffle %v2float %136 %136 0 1
%138 = OpFOrdEqual %v2bool %129 %137
%139 = OpAll %bool %138
OpBranch %128
%128 = OpLabel
%140 = OpPhi %bool %false %114 %139 %127
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%145 = OpLoad %v4float %144
%146 = OpVectorShuffle %v3float %145 %145 0 1 2
%147 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%148 = OpLoad %v4float %147
%149 = OpVectorShuffle %v3float %148 %148 0 1 2
%143 = OpExtInst %v3float %1 FMin %146 %149
%150 = OpLoad %v4float %expectedB
%151 = OpVectorShuffle %v3float %150 %150 0 1 2
%152 = OpFOrdEqual %v3bool %143 %151
%153 = OpAll %bool %152
OpBranch %142
%142 = OpLabel
%154 = OpPhi %bool %false %128 %153 %141
OpSelectionMerge %156 None
OpBranchConditional %154 %155 %156
%155 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%159 = OpLoad %v4float %158
%160 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%161 = OpLoad %v4float %160
%157 = OpExtInst %v4float %1 FMin %159 %161
%162 = OpLoad %v4float %expectedB
%163 = OpFOrdEqual %v4bool %157 %162
%164 = OpAll %bool %163
OpBranch %156
%156 = OpLabel
%165 = OpPhi %bool %false %142 %164 %155
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%168 = OpLoad %v4float %expectedB
%169 = OpCompositeExtract %float %168 0
%170 = OpFOrdEqual %bool %float_n1_25 %169
OpBranch %167
%167 = OpLabel
%171 = OpPhi %bool %false %156 %170 %166
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%174 = OpLoad %v4float %expectedB
%175 = OpVectorShuffle %v2float %174 %174 0 1
%176 = OpFOrdEqual %v2bool %93 %175
%177 = OpAll %bool %176
OpBranch %173
%173 = OpLabel
%178 = OpPhi %bool %false %167 %177 %172
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%182 = OpLoad %v4float %expectedB
%183 = OpVectorShuffle %v3float %182 %182 0 1 2
%184 = OpFOrdEqual %v3bool %181 %183
%185 = OpAll %bool %184
OpBranch %180
%180 = OpLabel
%186 = OpPhi %bool %false %173 %185 %179
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpLoad %v4float %expectedB
%190 = OpFOrdEqual %v4bool %35 %189
%191 = OpAll %bool %190
OpBranch %188
%188 = OpLabel
%192 = OpPhi %bool %false %180 %191 %187
OpSelectionMerge %196 None
OpBranchConditional %192 %194 %195
%194 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%198 = OpLoad %v4float %197
OpStore %193 %198
OpBranch %196
%195 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%201 = OpLoad %v4float %199
OpStore %193 %201
OpBranch %196
%196 = OpLabel
%202 = OpLoad %v4float %193
OpReturnValue %202
OpFunctionEnd
