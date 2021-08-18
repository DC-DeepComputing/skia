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
OpName %intValues "intValues"
OpName %intGreen "intGreen"
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
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%int_n125 = OpConstant %int -125
%int_50 = OpConstant %int 50
%64 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%int_100 = OpConstant %int 100
%67 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%119 = OpConstantComposite %v2int %int_n125 %int_0
%127 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
%198 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%intValues = OpVariable %_ptr_Function_v4int Function
%intGreen = OpVariable %_ptr_Function_v4int Function
%expectedA = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%210 = OpVariable %_ptr_Function_v4float Function
%32 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%35 = OpLoad %v4float %32
%37 = OpVectorTimesScalar %v4float %35 %float_100
%38 = OpCompositeExtract %float %37 0
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %37 1
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %37 2
%43 = OpConvertFToS %int %42
%44 = OpCompositeExtract %float %37 3
%45 = OpConvertFToS %int %44
%46 = OpCompositeConstruct %v4int %39 %41 %43 %45
OpStore %intValues %46
%48 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%50 = OpLoad %v4float %48
%51 = OpVectorTimesScalar %v4float %50 %float_100
%52 = OpCompositeExtract %float %51 0
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %51 1
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %51 2
%57 = OpConvertFToS %int %56
%58 = OpCompositeExtract %float %51 3
%59 = OpConvertFToS %int %58
%60 = OpCompositeConstruct %v4int %53 %55 %57 %59
OpStore %intGreen %60
OpStore %expectedA %64
OpStore %expectedB %67
%70 = OpLoad %v4int %intValues
%71 = OpCompositeExtract %int %70 0
%69 = OpExtInst %int %1 SMin %71 %int_50
%72 = OpLoad %v4int %expectedA
%73 = OpCompositeExtract %int %72 0
%74 = OpIEqual %bool %69 %73
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpLoad %v4int %intValues
%79 = OpVectorShuffle %v2int %78 %78 0 1
%81 = OpCompositeConstruct %v2int %int_50 %int_50
%77 = OpExtInst %v2int %1 SMin %79 %81
%82 = OpLoad %v4int %expectedA
%83 = OpVectorShuffle %v2int %82 %82 0 1
%84 = OpIEqual %v2bool %77 %83
%86 = OpAll %bool %84
OpBranch %76
%76 = OpLabel
%87 = OpPhi %bool %false %27 %86 %75
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%91 = OpLoad %v4int %intValues
%92 = OpVectorShuffle %v3int %91 %91 0 1 2
%94 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%90 = OpExtInst %v3int %1 SMin %92 %94
%95 = OpLoad %v4int %expectedA
%96 = OpVectorShuffle %v3int %95 %95 0 1 2
%97 = OpIEqual %v3bool %90 %96
%99 = OpAll %bool %97
OpBranch %89
%89 = OpLabel
%100 = OpPhi %bool %false %76 %99 %88
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpLoad %v4int %intValues
%105 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%103 = OpExtInst %v4int %1 SMin %104 %105
%106 = OpLoad %v4int %expectedA
%107 = OpIEqual %v4bool %103 %106
%109 = OpAll %bool %107
OpBranch %102
%102 = OpLabel
%110 = OpPhi %bool %false %89 %109 %101
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpLoad %v4int %expectedA
%114 = OpCompositeExtract %int %113 0
%115 = OpIEqual %bool %int_n125 %114
OpBranch %112
%112 = OpLabel
%116 = OpPhi %bool %false %102 %115 %111
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%120 = OpLoad %v4int %expectedA
%121 = OpVectorShuffle %v2int %120 %120 0 1
%122 = OpIEqual %v2bool %119 %121
%123 = OpAll %bool %122
OpBranch %118
%118 = OpLabel
%124 = OpPhi %bool %false %112 %123 %117
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpLoad %v4int %expectedA
%129 = OpVectorShuffle %v3int %128 %128 0 1 2
%130 = OpIEqual %v3bool %127 %129
%131 = OpAll %bool %130
OpBranch %126
%126 = OpLabel
%132 = OpPhi %bool %false %118 %131 %125
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpLoad %v4int %expectedA
%136 = OpIEqual %v4bool %64 %135
%137 = OpAll %bool %136
OpBranch %134
%134 = OpLabel
%138 = OpPhi %bool %false %126 %137 %133
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpLoad %v4int %intValues
%143 = OpCompositeExtract %int %142 0
%144 = OpLoad %v4int %intGreen
%145 = OpCompositeExtract %int %144 0
%141 = OpExtInst %int %1 SMin %143 %145
%146 = OpLoad %v4int %expectedB
%147 = OpCompositeExtract %int %146 0
%148 = OpIEqual %bool %141 %147
OpBranch %140
%140 = OpLabel
%149 = OpPhi %bool %false %134 %148 %139
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%153 = OpLoad %v4int %intValues
%154 = OpVectorShuffle %v2int %153 %153 0 1
%155 = OpLoad %v4int %intGreen
%156 = OpVectorShuffle %v2int %155 %155 0 1
%152 = OpExtInst %v2int %1 SMin %154 %156
%157 = OpLoad %v4int %expectedB
%158 = OpVectorShuffle %v2int %157 %157 0 1
%159 = OpIEqual %v2bool %152 %158
%160 = OpAll %bool %159
OpBranch %151
%151 = OpLabel
%161 = OpPhi %bool %false %140 %160 %150
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%165 = OpLoad %v4int %intValues
%166 = OpVectorShuffle %v3int %165 %165 0 1 2
%167 = OpLoad %v4int %intGreen
%168 = OpVectorShuffle %v3int %167 %167 0 1 2
%164 = OpExtInst %v3int %1 SMin %166 %168
%169 = OpLoad %v4int %expectedB
%170 = OpVectorShuffle %v3int %169 %169 0 1 2
%171 = OpIEqual %v3bool %164 %170
%172 = OpAll %bool %171
OpBranch %163
%163 = OpLabel
%173 = OpPhi %bool %false %151 %172 %162
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%177 = OpLoad %v4int %intValues
%178 = OpLoad %v4int %intGreen
%176 = OpExtInst %v4int %1 SMin %177 %178
%179 = OpLoad %v4int %expectedB
%180 = OpIEqual %v4bool %176 %179
%181 = OpAll %bool %180
OpBranch %175
%175 = OpLabel
%182 = OpPhi %bool %false %163 %181 %174
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %v4int %expectedB
%186 = OpCompositeExtract %int %185 0
%187 = OpIEqual %bool %int_n125 %186
OpBranch %184
%184 = OpLabel
%188 = OpPhi %bool %false %175 %187 %183
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpLoad %v4int %expectedB
%192 = OpVectorShuffle %v2int %191 %191 0 1
%193 = OpIEqual %v2bool %119 %192
%194 = OpAll %bool %193
OpBranch %190
%190 = OpLabel
%195 = OpPhi %bool %false %184 %194 %189
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%199 = OpLoad %v4int %expectedB
%200 = OpVectorShuffle %v3int %199 %199 0 1 2
%201 = OpIEqual %v3bool %198 %200
%202 = OpAll %bool %201
OpBranch %197
%197 = OpLabel
%203 = OpPhi %bool %false %190 %202 %196
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpLoad %v4int %expectedB
%207 = OpIEqual %v4bool %67 %206
%208 = OpAll %bool %207
OpBranch %205
%205 = OpLabel
%209 = OpPhi %bool %false %197 %208 %204
OpSelectionMerge %214 None
OpBranchConditional %209 %212 %213
%212 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%216 = OpLoad %v4float %215
OpStore %210 %216
OpBranch %214
%213 = OpLabel
%217 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%219 = OpLoad %v4float %217
OpStore %210 %219
OpBranch %214
%214 = OpLabel
%220 = OpLoad %v4float %210
OpReturnValue %220
OpFunctionEnd
