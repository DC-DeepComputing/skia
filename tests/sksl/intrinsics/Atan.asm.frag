OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputVal"
OpMemberName %_UniformBuffer 1 "expected"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal2 "constVal2"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %constVal2 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
%29 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%90 = OpConstantComposite %v2float %float_0 %float_0
%99 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%108 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%131 = OpConstantComposite %v2float %float_1 %float_1
%144 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%constVal2 = OpVariable %_ptr_Function_v4float Function
%192 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal2 %29
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpExtInst %float %1 Atan %37
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%40 = OpLoad %v4float %38
%41 = OpCompositeExtract %float %40 0
%42 = OpFOrdEqual %bool %31 %41
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v2float %47 %47 0 1
%45 = OpExtInst %v2float %1 Atan %48
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpFOrdEqual %v2bool %45 %51
%54 = OpAll %bool %52
OpBranch %44
%44 = OpLabel
%55 = OpPhi %bool %false %25 %54 %43
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%60 = OpLoad %v4float %59
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%58 = OpExtInst %v3float %1 Atan %61
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%66 = OpFOrdEqual %v3bool %58 %65
%68 = OpAll %bool %66
OpBranch %57
%57 = OpLabel
%69 = OpPhi %bool %false %44 %68 %56
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %73
%72 = OpExtInst %v4float %1 Atan %74
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%76 = OpLoad %v4float %75
%77 = OpFOrdEqual %v4bool %72 %76
%79 = OpAll %bool %77
OpBranch %71
%71 = OpLabel
%80 = OpPhi %bool %false %57 %79 %70
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%84 = OpLoad %v4float %83
%85 = OpCompositeExtract %float %84 0
%86 = OpFOrdEqual %bool %float_0 %85
OpBranch %82
%82 = OpLabel
%87 = OpPhi %bool %false %71 %86 %81
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%92 = OpLoad %v4float %91
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpFOrdEqual %v2bool %90 %93
%95 = OpAll %bool %94
OpBranch %89
%89 = OpLabel
%96 = OpPhi %bool %false %82 %95 %88
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpFOrdEqual %v3bool %99 %102
%104 = OpAll %bool %103
OpBranch %98
%98 = OpLabel
%105 = OpPhi %bool %false %89 %104 %97
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%110 = OpLoad %v4float %109
%111 = OpFOrdEqual %v4bool %108 %110
%112 = OpAll %bool %111
OpBranch %107
%107 = OpLabel
%113 = OpPhi %bool %false %98 %112 %106
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%116 = OpExtInst %float %1 Atan2 %119 %float_1
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 0
%123 = OpFOrdEqual %bool %116 %122
OpBranch %115
%115 = OpLabel
%124 = OpPhi %bool %false %107 %123 %114
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%129 = OpLoad %v4float %128
%130 = OpVectorShuffle %v2float %129 %129 0 1
%127 = OpExtInst %v2float %1 Atan2 %130 %131
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%133 = OpLoad %v4float %132
%134 = OpVectorShuffle %v2float %133 %133 0 1
%135 = OpFOrdEqual %v2bool %127 %134
%136 = OpAll %bool %135
OpBranch %126
%126 = OpLabel
%137 = OpPhi %bool %false %115 %136 %125
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%142 = OpLoad %v4float %141
%143 = OpVectorShuffle %v3float %142 %142 0 1 2
%140 = OpExtInst %v3float %1 Atan2 %143 %144
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%146 = OpLoad %v4float %145
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%148 = OpFOrdEqual %v3bool %140 %147
%149 = OpAll %bool %148
OpBranch %139
%139 = OpLabel
%150 = OpPhi %bool %false %126 %149 %138
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%155 = OpLoad %v4float %154
%156 = OpLoad %v4float %constVal2
%153 = OpExtInst %v4float %1 Atan2 %155 %156
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%158 = OpLoad %v4float %157
%159 = OpFOrdEqual %v4bool %153 %158
%160 = OpAll %bool %159
OpBranch %152
%152 = OpLabel
%161 = OpPhi %bool %false %139 %160 %151
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 0
%167 = OpFOrdEqual %bool %float_0 %166
OpBranch %163
%163 = OpLabel
%168 = OpPhi %bool %false %152 %167 %162
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%172 = OpLoad %v4float %171
%173 = OpVectorShuffle %v2float %172 %172 0 1
%174 = OpFOrdEqual %v2bool %90 %173
%175 = OpAll %bool %174
OpBranch %170
%170 = OpLabel
%176 = OpPhi %bool %false %163 %175 %169
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%180 = OpLoad %v4float %179
%181 = OpVectorShuffle %v3float %180 %180 0 1 2
%182 = OpFOrdEqual %v3bool %99 %181
%183 = OpAll %bool %182
OpBranch %178
%178 = OpLabel
%184 = OpPhi %bool %false %170 %183 %177
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%188 = OpLoad %v4float %187
%189 = OpFOrdEqual %v4bool %108 %188
%190 = OpAll %bool %189
OpBranch %186
%186 = OpLabel
%191 = OpPhi %bool %false %178 %190 %185
OpSelectionMerge %195 None
OpBranchConditional %191 %193 %194
%193 = OpLabel
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%198 = OpLoad %v4float %196
OpStore %192 %198
OpBranch %195
%194 = OpLabel
%199 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%201 = OpLoad %v4float %199
OpStore %192 %201
OpBranch %195
%195 = OpLabel
%202 = OpLoad %v4float %192
OpReturnValue %202
OpFunctionEnd
