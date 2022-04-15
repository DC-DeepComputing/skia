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
OpName %intValues "intValues"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_n100 = OpConstant %int -100
%int_75 = OpConstant %int 75
%int_100 = OpConstant %int 100
%49 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%int_n200 = OpConstant %int -200
%52 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_50 = OpConstant %int 50
%int_225 = OpConstant %int 225
%56 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
%int_200 = OpConstant %int 200
%int_300 = OpConstant %int 300
%60 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%71 = OpConstantComposite %v2int %int_n100 %int_n100
%72 = OpConstantComposite %v2int %int_100 %int_100
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%85 = OpConstantComposite %v3int %int_n100 %int_n100 %int_n100
%86 = OpConstantComposite %v3int %int_100 %int_100 %int_100
%v3bool = OpTypeVector %bool 3
%97 = OpConstantComposite %v4int %int_n100 %int_n100 %int_n100 %int_n100
%98 = OpConstantComposite %v4int %int_100 %int_100 %int_100 %int_100
%v4bool = OpTypeVector %bool 4
%112 = OpConstantComposite %v2int %int_n100 %int_0
%120 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
%146 = OpConstantComposite %v2int %int_n100 %int_n200
%147 = OpConstantComposite %v2int %int_100 %int_200
%158 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
%159 = OpConstantComposite %v3int %int_100 %int_200 %int_50
%190 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%intValues = OpVariable %_ptr_Function_v4int Function
%expectedA = OpVariable %_ptr_Function_v4int Function
%clampLow = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%clampHigh = OpVariable %_ptr_Function_v4int Function
%202 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %30
%35 = OpVectorTimesScalar %v4float %33 %float_100
%36 = OpCompositeExtract %float %35 0
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %35 1
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %35 2
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %35 3
%43 = OpConvertFToS %int %42
%44 = OpCompositeConstruct %v4int %37 %39 %41 %43
OpStore %intValues %44
OpStore %expectedA %49
OpStore %clampLow %52
OpStore %expectedB %56
OpStore %clampHigh %60
%63 = OpCompositeExtract %int %44 0
%62 = OpExtInst %int %1 SClamp %63 %int_n100 %int_100
%64 = OpIEqual %bool %62 %int_n100
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpLoad %v4int %intValues
%69 = OpVectorShuffle %v2int %68 %68 0 1
%67 = OpExtInst %v2int %1 SClamp %69 %71 %72
%73 = OpLoad %v4int %expectedA
%74 = OpVectorShuffle %v2int %73 %73 0 1
%75 = OpIEqual %v2bool %67 %74
%77 = OpAll %bool %75
OpBranch %66
%66 = OpLabel
%78 = OpPhi %bool %false %25 %77 %65
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%82 = OpLoad %v4int %intValues
%83 = OpVectorShuffle %v3int %82 %82 0 1 2
%81 = OpExtInst %v3int %1 SClamp %83 %85 %86
%87 = OpLoad %v4int %expectedA
%88 = OpVectorShuffle %v3int %87 %87 0 1 2
%89 = OpIEqual %v3bool %81 %88
%91 = OpAll %bool %89
OpBranch %80
%80 = OpLabel
%92 = OpPhi %bool %false %66 %91 %79
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v4int %intValues
%95 = OpExtInst %v4int %1 SClamp %96 %97 %98
%99 = OpLoad %v4int %expectedA
%100 = OpIEqual %v4bool %95 %99
%102 = OpAll %bool %100
OpBranch %94
%94 = OpLabel
%103 = OpPhi %bool %false %80 %102 %93
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %v4int %expectedA
%107 = OpCompositeExtract %int %106 0
%108 = OpIEqual %bool %int_n100 %107
OpBranch %105
%105 = OpLabel
%109 = OpPhi %bool %false %94 %108 %104
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpLoad %v4int %expectedA
%114 = OpVectorShuffle %v2int %113 %113 0 1
%115 = OpIEqual %v2bool %112 %114
%116 = OpAll %bool %115
OpBranch %111
%111 = OpLabel
%117 = OpPhi %bool %false %105 %116 %110
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v4int %expectedA
%122 = OpVectorShuffle %v3int %121 %121 0 1 2
%123 = OpIEqual %v3bool %120 %122
%124 = OpAll %bool %123
OpBranch %119
%119 = OpLabel
%125 = OpPhi %bool %false %111 %124 %118
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpLoad %v4int %expectedA
%129 = OpIEqual %v4bool %49 %128
%130 = OpAll %bool %129
OpBranch %127
%127 = OpLabel
%131 = OpPhi %bool %false %119 %130 %126
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%135 = OpLoad %v4int %intValues
%136 = OpCompositeExtract %int %135 0
%134 = OpExtInst %int %1 SClamp %136 %int_n100 %int_100
%137 = OpLoad %v4int %expectedB
%138 = OpCompositeExtract %int %137 0
%139 = OpIEqual %bool %134 %138
OpBranch %133
%133 = OpLabel
%140 = OpPhi %bool %false %127 %139 %132
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpLoad %v4int %intValues
%145 = OpVectorShuffle %v2int %144 %144 0 1
%143 = OpExtInst %v2int %1 SClamp %145 %146 %147
%148 = OpLoad %v4int %expectedB
%149 = OpVectorShuffle %v2int %148 %148 0 1
%150 = OpIEqual %v2bool %143 %149
%151 = OpAll %bool %150
OpBranch %142
%142 = OpLabel
%152 = OpPhi %bool %false %133 %151 %141
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%156 = OpLoad %v4int %intValues
%157 = OpVectorShuffle %v3int %156 %156 0 1 2
%155 = OpExtInst %v3int %1 SClamp %157 %158 %159
%160 = OpLoad %v4int %expectedB
%161 = OpVectorShuffle %v3int %160 %160 0 1 2
%162 = OpIEqual %v3bool %155 %161
%163 = OpAll %bool %162
OpBranch %154
%154 = OpLabel
%164 = OpPhi %bool %false %142 %163 %153
OpSelectionMerge %166 None
OpBranchConditional %164 %165 %166
%165 = OpLabel
%168 = OpLoad %v4int %intValues
%169 = OpLoad %v4int %clampLow
%170 = OpLoad %v4int %clampHigh
%167 = OpExtInst %v4int %1 SClamp %168 %169 %170
%171 = OpLoad %v4int %expectedB
%172 = OpIEqual %v4bool %167 %171
%173 = OpAll %bool %172
OpBranch %166
%166 = OpLabel
%174 = OpPhi %bool %false %154 %173 %165
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%177 = OpLoad %v4int %expectedB
%178 = OpCompositeExtract %int %177 0
%179 = OpIEqual %bool %int_n100 %178
OpBranch %176
%176 = OpLabel
%180 = OpPhi %bool %false %166 %179 %175
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%183 = OpLoad %v4int %expectedB
%184 = OpVectorShuffle %v2int %183 %183 0 1
%185 = OpIEqual %v2bool %112 %184
%186 = OpAll %bool %185
OpBranch %182
%182 = OpLabel
%187 = OpPhi %bool %false %176 %186 %181
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%191 = OpLoad %v4int %expectedB
%192 = OpVectorShuffle %v3int %191 %191 0 1 2
%193 = OpIEqual %v3bool %190 %192
%194 = OpAll %bool %193
OpBranch %189
%189 = OpLabel
%195 = OpPhi %bool %false %182 %194 %188
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%198 = OpLoad %v4int %expectedB
%199 = OpIEqual %v4bool %56 %198
%200 = OpAll %bool %199
OpBranch %197
%197 = OpLabel
%201 = OpPhi %bool %false %189 %200 %196
OpSelectionMerge %206 None
OpBranchConditional %201 %204 %205
%204 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%209 = OpLoad %v4float %207
OpStore %202 %209
OpBranch %206
%205 = OpLabel
%210 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%212 = OpLoad %v4float %210
OpStore %202 %212
OpBranch %206
%206 = OpLabel
%213 = OpLoad %v4float %202
OpReturnValue %213
OpFunctionEnd
