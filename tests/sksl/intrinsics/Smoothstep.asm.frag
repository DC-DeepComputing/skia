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
OpName %constVal "constVal"
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
OpDecorate %constVal RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
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
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%float_0_84375 = OpConstant %float 0.84375
%float_1 = OpConstant %float 1
%35 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
%37 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%50 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%108 = OpConstantComposite %v2float %float_n1_25 %float_0
%124 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
%149 = OpConstantComposite %v2float %float_1 %float_0
%156 = OpConstantComposite %v3float %float_1 %float_0 %float_1
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
%constVal = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%213 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %31
OpStore %expectedA %35
OpStore %expectedB %37
%39 = OpFOrdEqual %bool %float_0 %float_0
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpVectorShuffle %v2float %35 %35 0 1
%44 = OpFOrdEqual %v2bool %19 %42
%45 = OpAll %bool %44
OpBranch %41
%41 = OpLabel
%46 = OpPhi %bool %false %25 %45 %40
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%51 = OpVectorShuffle %v3float %35 %35 0 1 2
%53 = OpFOrdEqual %v3bool %50 %51
%54 = OpAll %bool %53
OpBranch %48
%48 = OpLabel
%55 = OpPhi %bool %false %41 %54 %47
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpFOrdEqual %v4bool %35 %35
%60 = OpAll %bool %59
OpBranch %57
%57 = OpLabel
%61 = OpPhi %bool %false %48 %60 %56
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
OpBranch %63
%63 = OpLabel
%64 = OpPhi %bool %false %57 %39 %62
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpVectorShuffle %v2float %35 %35 0 1
%68 = OpFOrdEqual %v2bool %19 %67
%69 = OpAll %bool %68
OpBranch %66
%66 = OpLabel
%70 = OpPhi %bool %false %63 %69 %65
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpVectorShuffle %v3float %35 %35 0 1 2
%74 = OpFOrdEqual %v3bool %50 %73
%75 = OpAll %bool %74
OpBranch %72
%72 = OpLabel
%76 = OpPhi %bool %false %66 %75 %71
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpFOrdEqual %v4bool %35 %35
%80 = OpAll %bool %79
OpBranch %78
%78 = OpLabel
%81 = OpPhi %bool %false %72 %80 %77
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%89 = OpLoad %v4float %85
%90 = OpCompositeExtract %float %89 1
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%93 = OpLoad %v4float %91
%94 = OpCompositeExtract %float %93 1
%84 = OpExtInst %float %1 SmoothStep %90 %94 %float_n1_25
%95 = OpFOrdEqual %bool %84 %float_0
OpBranch %83
%83 = OpLabel
%96 = OpPhi %bool %false %78 %95 %82
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%101 = OpLoad %v4float %100
%102 = OpCompositeExtract %float %101 1
%103 = OpCompositeConstruct %v2float %102 %102
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %104
%106 = OpCompositeExtract %float %105 1
%107 = OpCompositeConstruct %v2float %106 %106
%99 = OpExtInst %v2float %1 SmoothStep %103 %107 %108
%109 = OpVectorShuffle %v2float %35 %35 0 1
%110 = OpFOrdEqual %v2bool %99 %109
%111 = OpAll %bool %110
OpBranch %98
%98 = OpLabel
%112 = OpPhi %bool %false %83 %111 %97
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%117 = OpLoad %v4float %116
%118 = OpCompositeExtract %float %117 1
%119 = OpCompositeConstruct %v3float %118 %118 %118
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%121 = OpLoad %v4float %120
%122 = OpCompositeExtract %float %121 1
%123 = OpCompositeConstruct %v3float %122 %122 %122
%115 = OpExtInst %v3float %1 SmoothStep %119 %123 %124
%125 = OpVectorShuffle %v3float %35 %35 0 1 2
%126 = OpFOrdEqual %v3bool %115 %125
%127 = OpAll %bool %126
OpBranch %114
%114 = OpLabel
%128 = OpPhi %bool %false %98 %127 %113
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%133 = OpLoad %v4float %132
%134 = OpCompositeExtract %float %133 1
%135 = OpCompositeConstruct %v4float %134 %134 %134 %134
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 1
%139 = OpCompositeConstruct %v4float %138 %138 %138 %138
%131 = OpExtInst %v4float %1 SmoothStep %135 %139 %31
%140 = OpFOrdEqual %v4bool %131 %35
%141 = OpAll %bool %140
OpBranch %130
%130 = OpLabel
%142 = OpPhi %bool %false %114 %141 %129
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%145 = OpFOrdEqual %bool %float_1 %float_1
OpBranch %144
%144 = OpLabel
%146 = OpPhi %bool %false %130 %145 %143
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%150 = OpVectorShuffle %v2float %37 %37 0 1
%151 = OpFOrdEqual %v2bool %149 %150
%152 = OpAll %bool %151
OpBranch %148
%148 = OpLabel
%153 = OpPhi %bool %false %144 %152 %147
OpSelectionMerge %155 None
OpBranchConditional %153 %154 %155
%154 = OpLabel
%157 = OpVectorShuffle %v3float %37 %37 0 1 2
%158 = OpFOrdEqual %v3bool %156 %157
%159 = OpAll %bool %158
OpBranch %155
%155 = OpLabel
%160 = OpPhi %bool %false %148 %159 %154
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%163 = OpFOrdEqual %v4bool %37 %37
%164 = OpAll %bool %163
OpBranch %162
%162 = OpLabel
%165 = OpPhi %bool %false %155 %164 %161
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%169 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%170 = OpLoad %v4float %169
%171 = OpCompositeExtract %float %170 0
%172 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%173 = OpLoad %v4float %172
%174 = OpCompositeExtract %float %173 0
%168 = OpExtInst %float %1 SmoothStep %171 %174 %float_n1_25
%175 = OpFOrdEqual %bool %168 %float_1
OpBranch %167
%167 = OpLabel
%176 = OpPhi %bool %false %162 %175 %166
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%181 = OpLoad %v4float %180
%182 = OpVectorShuffle %v2float %181 %181 0 1
%183 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%184 = OpLoad %v4float %183
%185 = OpVectorShuffle %v2float %184 %184 0 1
%179 = OpExtInst %v2float %1 SmoothStep %182 %185 %108
%186 = OpVectorShuffle %v2float %37 %37 0 1
%187 = OpFOrdEqual %v2bool %179 %186
%188 = OpAll %bool %187
OpBranch %178
%178 = OpLabel
%189 = OpPhi %bool %false %167 %188 %177
OpSelectionMerge %191 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%194 = OpLoad %v4float %193
%195 = OpVectorShuffle %v3float %194 %194 0 1 2
%196 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%197 = OpLoad %v4float %196
%198 = OpVectorShuffle %v3float %197 %197 0 1 2
%192 = OpExtInst %v3float %1 SmoothStep %195 %198 %124
%199 = OpVectorShuffle %v3float %37 %37 0 1 2
%200 = OpFOrdEqual %v3bool %192 %199
%201 = OpAll %bool %200
OpBranch %191
%191 = OpLabel
%202 = OpPhi %bool %false %178 %201 %190
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%207 = OpLoad %v4float %206
%208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%209 = OpLoad %v4float %208
%205 = OpExtInst %v4float %1 SmoothStep %207 %209 %31
%210 = OpFOrdEqual %v4bool %205 %37
%211 = OpAll %bool %210
OpBranch %204
%204 = OpLabel
%212 = OpPhi %bool %false %191 %211 %203
OpSelectionMerge %216 None
OpBranchConditional %212 %214 %215
%214 = OpLabel
%217 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%218 = OpLoad %v4float %217
OpStore %213 %218
OpBranch %216
%215 = OpLabel
%219 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%220 = OpLoad %v4float %219
OpStore %213 %220
OpBranch %216
%216 = OpLabel
%221 = OpLoad %v4float %213
OpReturnValue %221
OpFunctionEnd
