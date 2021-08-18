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
OpName %constGreen "constGreen"
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
OpDecorate %constGreen RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%31 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%33 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
%35 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_0
%false = OpConstantFalse %bool
%float_0_5 = OpConstant %float 0.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%101 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%129 = OpConstantComposite %v2float %float_0 %float_1
%141 = OpConstantComposite %v3float %float_0 %float_1 %float_0
%165 = OpConstantComposite %v2float %float_1 %float_1
%173 = OpConstantComposite %v3float %float_1 %float_1 %float_0
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
%constGreen = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%185 = OpVariable %_ptr_Function_v4float Function
OpStore %constGreen %31
OpStore %expectedA %33
OpStore %expectedB %35
%39 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%43 = OpLoad %v4float %39
%44 = OpCompositeExtract %float %43 0
%37 = OpExtInst %float %1 Step %float_0_5 %44
%45 = OpLoad %v4float %expectedA
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %37 %46
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%51 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%52 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 0 1
%50 = OpExtInst %v2float %1 Step %51 %54
%55 = OpLoad %v4float %expectedA
%56 = OpVectorShuffle %v2float %55 %55 0 1
%57 = OpFOrdEqual %v2bool %50 %56
%59 = OpAll %bool %57
OpBranch %49
%49 = OpLabel
%60 = OpPhi %bool %false %27 %59 %48
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%66 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%63 = OpExtInst %v3float %1 Step %64 %68
%69 = OpLoad %v4float %expectedA
%70 = OpVectorShuffle %v3float %69 %69 0 1 2
%71 = OpFOrdEqual %v3bool %63 %70
%73 = OpAll %bool %71
OpBranch %62
%62 = OpLabel
%74 = OpPhi %bool %false %49 %73 %61
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%79 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%80 = OpLoad %v4float %79
%77 = OpExtInst %v4float %1 Step %78 %80
%81 = OpLoad %v4float %expectedA
%82 = OpFOrdEqual %v4bool %77 %81
%84 = OpAll %bool %82
OpBranch %76
%76 = OpLabel
%85 = OpPhi %bool %false %62 %84 %75
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %v4float %expectedA
%89 = OpCompositeExtract %float %88 0
%90 = OpFOrdEqual %bool %float_0 %89
OpBranch %87
%87 = OpLabel
%91 = OpPhi %bool %false %76 %90 %86
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %v4float %expectedA
%95 = OpVectorShuffle %v2float %94 %94 0 1
%96 = OpFOrdEqual %v2bool %21 %95
%97 = OpAll %bool %96
OpBranch %93
%93 = OpLabel
%98 = OpPhi %bool %false %87 %97 %92
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpLoad %v4float %expectedA
%103 = OpVectorShuffle %v3float %102 %102 0 1 2
%104 = OpFOrdEqual %v3bool %101 %103
%105 = OpAll %bool %104
OpBranch %100
%100 = OpLabel
%106 = OpPhi %bool %false %93 %105 %99
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4float %expectedA
%110 = OpFOrdEqual %v4bool %33 %109
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
%115 = OpExtInst %float %1 Step %118 %float_0
%119 = OpLoad %v4float %expectedB
%120 = OpCompositeExtract %float %119 0
%121 = OpFOrdEqual %bool %115 %120
OpBranch %114
%114 = OpLabel
%122 = OpPhi %bool %false %108 %121 %113
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%127 = OpLoad %v4float %126
%128 = OpVectorShuffle %v2float %127 %127 0 1
%125 = OpExtInst %v2float %1 Step %128 %129
%130 = OpLoad %v4float %expectedB
%131 = OpVectorShuffle %v2float %130 %130 0 1
%132 = OpFOrdEqual %v2bool %125 %131
%133 = OpAll %bool %132
OpBranch %124
%124 = OpLabel
%134 = OpPhi %bool %false %114 %133 %123
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%139 = OpLoad %v4float %138
%140 = OpVectorShuffle %v3float %139 %139 0 1 2
%137 = OpExtInst %v3float %1 Step %140 %141
%142 = OpLoad %v4float %expectedB
%143 = OpVectorShuffle %v3float %142 %142 0 1 2
%144 = OpFOrdEqual %v3bool %137 %143
%145 = OpAll %bool %144
OpBranch %136
%136 = OpLabel
%146 = OpPhi %bool %false %124 %145 %135
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%151 = OpLoad %v4float %150
%152 = OpLoad %v4float %constGreen
%149 = OpExtInst %v4float %1 Step %151 %152
%153 = OpLoad %v4float %expectedB
%154 = OpFOrdEqual %v4bool %149 %153
%155 = OpAll %bool %154
OpBranch %148
%148 = OpLabel
%156 = OpPhi %bool %false %136 %155 %147
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%159 = OpLoad %v4float %expectedB
%160 = OpCompositeExtract %float %159 0
%161 = OpFOrdEqual %bool %float_1 %160
OpBranch %158
%158 = OpLabel
%162 = OpPhi %bool %false %148 %161 %157
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpLoad %v4float %expectedB
%167 = OpVectorShuffle %v2float %166 %166 0 1
%168 = OpFOrdEqual %v2bool %165 %167
%169 = OpAll %bool %168
OpBranch %164
%164 = OpLabel
%170 = OpPhi %bool %false %158 %169 %163
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%174 = OpLoad %v4float %expectedB
%175 = OpVectorShuffle %v3float %174 %174 0 1 2
%176 = OpFOrdEqual %v3bool %173 %175
%177 = OpAll %bool %176
OpBranch %172
%172 = OpLabel
%178 = OpPhi %bool %false %164 %177 %171
OpSelectionMerge %180 None
OpBranchConditional %178 %179 %180
%179 = OpLabel
%181 = OpLoad %v4float %expectedB
%182 = OpFOrdEqual %v4bool %35 %181
%183 = OpAll %bool %182
OpBranch %180
%180 = OpLabel
%184 = OpPhi %bool %false %172 %183 %179
OpSelectionMerge %188 None
OpBranchConditional %184 %186 %187
%186 = OpLabel
%189 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%191 = OpLoad %v4float %189
OpStore %185 %191
OpBranch %188
%187 = OpLabel
%192 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%194 = OpLoad %v4float %192
OpStore %185 %194
OpBranch %188
%188 = OpLabel
%195 = OpLoad %v4float %185
OpReturnValue %195
OpFunctionEnd
