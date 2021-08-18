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
OpDecorate %33 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%33 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_1 = OpConstant %float 1
%36 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%94 = OpConstantComposite %v2float %float_0_5 %float_0_5
%102 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
%int_1 = OpConstant %int 1
%175 = OpConstantComposite %v2float %float_0 %float_1
%183 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
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
%195 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %33
OpStore %expectedB %36
%39 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%43 = OpLoad %v4float %39
%44 = OpCompositeExtract %float %43 0
%38 = OpExtInst %float %1 FMax %44 %float_0_5
%45 = OpLoad %v4float %expectedA
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %38 %46
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
%54 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%50 = OpExtInst %v2float %1 FMax %53 %54
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
%64 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%68 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%63 = OpExtInst %v3float %1 FMax %66 %68
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
%78 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%79 = OpLoad %v4float %78
%80 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%77 = OpExtInst %v4float %1 FMax %79 %80
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
%90 = OpFOrdEqual %bool %float_0_5 %89
OpBranch %87
%87 = OpLabel
%91 = OpPhi %bool %false %76 %90 %86
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%95 = OpLoad %v4float %expectedA
%96 = OpVectorShuffle %v2float %95 %95 0 1
%97 = OpFOrdEqual %v2bool %94 %96
%98 = OpAll %bool %97
OpBranch %93
%93 = OpLabel
%99 = OpPhi %bool %false %87 %98 %92
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%103 = OpLoad %v4float %expectedA
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%105 = OpFOrdEqual %v3bool %102 %104
%106 = OpAll %bool %105
OpBranch %101
%101 = OpLabel
%107 = OpPhi %bool %false %93 %106 %100
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpLoad %v4float %expectedA
%111 = OpFOrdEqual %v4bool %33 %110
%112 = OpAll %bool %111
OpBranch %109
%109 = OpLabel
%113 = OpPhi %bool %false %101 %112 %108
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%120 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%122 = OpLoad %v4float %120
%123 = OpCompositeExtract %float %122 0
%116 = OpExtInst %float %1 FMax %119 %123
%124 = OpLoad %v4float %expectedB
%125 = OpCompositeExtract %float %124 0
%126 = OpFOrdEqual %bool %116 %125
OpBranch %115
%115 = OpLabel
%127 = OpPhi %bool %false %109 %126 %114
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%132 = OpLoad %v4float %131
%133 = OpVectorShuffle %v2float %132 %132 0 1
%134 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%135 = OpLoad %v4float %134
%136 = OpVectorShuffle %v2float %135 %135 0 1
%130 = OpExtInst %v2float %1 FMax %133 %136
%137 = OpLoad %v4float %expectedB
%138 = OpVectorShuffle %v2float %137 %137 0 1
%139 = OpFOrdEqual %v2bool %130 %138
%140 = OpAll %bool %139
OpBranch %129
%129 = OpLabel
%141 = OpPhi %bool %false %115 %140 %128
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%146 = OpLoad %v4float %145
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%148 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%149 = OpLoad %v4float %148
%150 = OpVectorShuffle %v3float %149 %149 0 1 2
%144 = OpExtInst %v3float %1 FMax %147 %150
%151 = OpLoad %v4float %expectedB
%152 = OpVectorShuffle %v3float %151 %151 0 1 2
%153 = OpFOrdEqual %v3bool %144 %152
%154 = OpAll %bool %153
OpBranch %143
%143 = OpLabel
%155 = OpPhi %bool %false %129 %154 %142
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%160 = OpLoad %v4float %159
%161 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%162 = OpLoad %v4float %161
%158 = OpExtInst %v4float %1 FMax %160 %162
%163 = OpLoad %v4float %expectedB
%164 = OpFOrdEqual %v4bool %158 %163
%165 = OpAll %bool %164
OpBranch %157
%157 = OpLabel
%166 = OpPhi %bool %false %143 %165 %156
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpLoad %v4float %expectedB
%170 = OpCompositeExtract %float %169 0
%171 = OpFOrdEqual %bool %float_0 %170
OpBranch %168
%168 = OpLabel
%172 = OpPhi %bool %false %157 %171 %167
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpLoad %v4float %expectedB
%177 = OpVectorShuffle %v2float %176 %176 0 1
%178 = OpFOrdEqual %v2bool %175 %177
%179 = OpAll %bool %178
OpBranch %174
%174 = OpLabel
%180 = OpPhi %bool %false %168 %179 %173
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%184 = OpLoad %v4float %expectedB
%185 = OpVectorShuffle %v3float %184 %184 0 1 2
%186 = OpFOrdEqual %v3bool %183 %185
%187 = OpAll %bool %186
OpBranch %182
%182 = OpLabel
%188 = OpPhi %bool %false %174 %187 %181
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %v4float %expectedB
%192 = OpFOrdEqual %v4bool %36 %191
%193 = OpAll %bool %192
OpBranch %190
%190 = OpLabel
%194 = OpPhi %bool %false %182 %193 %189
OpSelectionMerge %198 None
OpBranchConditional %194 %196 %197
%196 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%200 = OpLoad %v4float %199
OpStore %195 %200
OpBranch %198
%197 = OpLabel
%201 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%203 = OpLoad %v4float %201
OpStore %195 %203
OpBranch %198
%198 = OpLabel
%204 = OpLoad %v4float %195
OpReturnValue %204
OpFunctionEnd
