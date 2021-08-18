OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "input"
OpMemberName %_UniformBuffer 1 "expected"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal2 "constVal2"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %constVal2 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
%31 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%100 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%109 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%132 = OpConstantComposite %v2float %float_1 %float_1
%145 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%constVal2 = OpVariable %_ptr_Function_v4float Function
%193 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal2 %31
%34 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%38 = OpLoad %v4float %34
%39 = OpCompositeExtract %float %38 0
%33 = OpExtInst %float %1 Atan %39
%40 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%42 = OpLoad %v4float %40
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdEqual %bool %33 %43
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%47 = OpExtInst %v2float %1 Atan %50
%51 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
%54 = OpFOrdEqual %v2bool %47 %53
%56 = OpAll %bool %54
OpBranch %46
%46 = OpLabel
%57 = OpPhi %bool %false %27 %56 %45
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%60 = OpExtInst %v3float %1 Atan %63
%65 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%68 = OpFOrdEqual %v3bool %60 %67
%70 = OpAll %bool %68
OpBranch %59
%59 = OpLabel
%71 = OpPhi %bool %false %46 %70 %58
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%76 = OpLoad %v4float %75
%74 = OpExtInst %v4float %1 Atan %76
%77 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%78 = OpLoad %v4float %77
%79 = OpFOrdEqual %v4bool %74 %78
%81 = OpAll %bool %79
OpBranch %73
%73 = OpLabel
%82 = OpPhi %bool %false %59 %81 %72
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%86 = OpLoad %v4float %85
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdEqual %bool %float_0 %87
OpBranch %84
%84 = OpLabel
%89 = OpPhi %bool %false %73 %88 %83
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%93 = OpLoad %v4float %92
%94 = OpVectorShuffle %v2float %93 %93 0 1
%95 = OpFOrdEqual %v2bool %21 %94
%96 = OpAll %bool %95
OpBranch %91
%91 = OpLabel
%97 = OpPhi %bool %false %84 %96 %90
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%102 = OpLoad %v4float %101
%103 = OpVectorShuffle %v3float %102 %102 0 1 2
%104 = OpFOrdEqual %v3bool %100 %103
%105 = OpAll %bool %104
OpBranch %99
%99 = OpLabel
%106 = OpPhi %bool %false %91 %105 %98
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%111 = OpLoad %v4float %110
%112 = OpFOrdEqual %v4bool %109 %111
%113 = OpAll %bool %112
OpBranch %108
%108 = OpLabel
%114 = OpPhi %bool %false %99 %113 %107
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%119 = OpLoad %v4float %118
%120 = OpCompositeExtract %float %119 0
%117 = OpExtInst %float %1 Atan2 %120 %float_1
%121 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%122 = OpLoad %v4float %121
%123 = OpCompositeExtract %float %122 0
%124 = OpFOrdEqual %bool %117 %123
OpBranch %116
%116 = OpLabel
%125 = OpPhi %bool %false %108 %124 %115
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%130 = OpLoad %v4float %129
%131 = OpVectorShuffle %v2float %130 %130 0 1
%128 = OpExtInst %v2float %1 Atan2 %131 %132
%133 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%134 = OpLoad %v4float %133
%135 = OpVectorShuffle %v2float %134 %134 0 1
%136 = OpFOrdEqual %v2bool %128 %135
%137 = OpAll %bool %136
OpBranch %127
%127 = OpLabel
%138 = OpPhi %bool %false %116 %137 %126
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%143 = OpLoad %v4float %142
%144 = OpVectorShuffle %v3float %143 %143 0 1 2
%141 = OpExtInst %v3float %1 Atan2 %144 %145
%146 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%147 = OpLoad %v4float %146
%148 = OpVectorShuffle %v3float %147 %147 0 1 2
%149 = OpFOrdEqual %v3bool %141 %148
%150 = OpAll %bool %149
OpBranch %140
%140 = OpLabel
%151 = OpPhi %bool %false %127 %150 %139
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%156 = OpLoad %v4float %155
%157 = OpLoad %v4float %constVal2
%154 = OpExtInst %v4float %1 Atan2 %156 %157
%158 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%159 = OpLoad %v4float %158
%160 = OpFOrdEqual %v4bool %154 %159
%161 = OpAll %bool %160
OpBranch %153
%153 = OpLabel
%162 = OpPhi %bool %false %140 %161 %152
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%165 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%166 = OpLoad %v4float %165
%167 = OpCompositeExtract %float %166 0
%168 = OpFOrdEqual %bool %float_0 %167
OpBranch %164
%164 = OpLabel
%169 = OpPhi %bool %false %153 %168 %163
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%173 = OpLoad %v4float %172
%174 = OpVectorShuffle %v2float %173 %173 0 1
%175 = OpFOrdEqual %v2bool %21 %174
%176 = OpAll %bool %175
OpBranch %171
%171 = OpLabel
%177 = OpPhi %bool %false %164 %176 %170
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%181 = OpLoad %v4float %180
%182 = OpVectorShuffle %v3float %181 %181 0 1 2
%183 = OpFOrdEqual %v3bool %100 %182
%184 = OpAll %bool %183
OpBranch %179
%179 = OpLabel
%185 = OpPhi %bool %false %171 %184 %178
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%189 = OpLoad %v4float %188
%190 = OpFOrdEqual %v4bool %109 %189
%191 = OpAll %bool %190
OpBranch %187
%187 = OpLabel
%192 = OpPhi %bool %false %179 %191 %186
OpSelectionMerge %196 None
OpBranchConditional %192 %194 %195
%194 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%199 = OpLoad %v4float %197
OpStore %193 %199
OpBranch %196
%195 = OpLabel
%200 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%202 = OpLoad %v4float %200
OpStore %193 %202
OpBranch %196
%196 = OpLabel
%203 = OpLoad %v4float %193
OpReturnValue %203
OpFunctionEnd
