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
OpDecorate %218 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
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
%int_50 = OpConstant %int 50
%int_75 = OpConstant %int 75
%int_225 = OpConstant %int 225
%65 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
%int_100 = OpConstant %int 100
%68 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%120 = OpConstantComposite %v2int %int_50 %int_50
%128 = OpConstantComposite %v3int %int_50 %int_50 %int_75
%192 = OpConstantComposite %v2int %int_0 %int_100
%200 = OpConstantComposite %v3int %int_0 %int_100 %int_75
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
%212 = OpVariable %_ptr_Function_v4float Function
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
OpStore %expectedA %65
OpStore %expectedB %68
%71 = OpLoad %v4int %intValues
%72 = OpCompositeExtract %int %71 0
%70 = OpExtInst %int %1 SMax %72 %int_50
%73 = OpLoad %v4int %expectedA
%74 = OpCompositeExtract %int %73 0
%75 = OpIEqual %bool %70 %74
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpLoad %v4int %intValues
%80 = OpVectorShuffle %v2int %79 %79 0 1
%82 = OpCompositeConstruct %v2int %int_50 %int_50
%78 = OpExtInst %v2int %1 SMax %80 %82
%83 = OpLoad %v4int %expectedA
%84 = OpVectorShuffle %v2int %83 %83 0 1
%85 = OpIEqual %v2bool %78 %84
%87 = OpAll %bool %85
OpBranch %77
%77 = OpLabel
%88 = OpPhi %bool %false %27 %87 %76
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpLoad %v4int %intValues
%93 = OpVectorShuffle %v3int %92 %92 0 1 2
%95 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%91 = OpExtInst %v3int %1 SMax %93 %95
%96 = OpLoad %v4int %expectedA
%97 = OpVectorShuffle %v3int %96 %96 0 1 2
%98 = OpIEqual %v3bool %91 %97
%100 = OpAll %bool %98
OpBranch %90
%90 = OpLabel
%101 = OpPhi %bool %false %77 %100 %89
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%105 = OpLoad %v4int %intValues
%106 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%104 = OpExtInst %v4int %1 SMax %105 %106
%107 = OpLoad %v4int %expectedA
%108 = OpIEqual %v4bool %104 %107
%110 = OpAll %bool %108
OpBranch %103
%103 = OpLabel
%111 = OpPhi %bool %false %90 %110 %102
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%114 = OpLoad %v4int %expectedA
%115 = OpCompositeExtract %int %114 0
%116 = OpIEqual %bool %int_50 %115
OpBranch %113
%113 = OpLabel
%117 = OpPhi %bool %false %103 %116 %112
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v4int %expectedA
%122 = OpVectorShuffle %v2int %121 %121 0 1
%123 = OpIEqual %v2bool %120 %122
%124 = OpAll %bool %123
OpBranch %119
%119 = OpLabel
%125 = OpPhi %bool %false %113 %124 %118
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpLoad %v4int %expectedA
%130 = OpVectorShuffle %v3int %129 %129 0 1 2
%131 = OpIEqual %v3bool %128 %130
%132 = OpAll %bool %131
OpBranch %127
%127 = OpLabel
%133 = OpPhi %bool %false %119 %132 %126
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpLoad %v4int %expectedA
%137 = OpIEqual %v4bool %65 %136
%138 = OpAll %bool %137
OpBranch %135
%135 = OpLabel
%139 = OpPhi %bool %false %127 %138 %134
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%143 = OpLoad %v4int %intValues
%144 = OpCompositeExtract %int %143 0
%145 = OpLoad %v4int %intGreen
%146 = OpCompositeExtract %int %145 0
%142 = OpExtInst %int %1 SMax %144 %146
%147 = OpLoad %v4int %expectedB
%148 = OpCompositeExtract %int %147 0
%149 = OpIEqual %bool %142 %148
OpBranch %141
%141 = OpLabel
%150 = OpPhi %bool %false %135 %149 %140
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%154 = OpLoad %v4int %intValues
%155 = OpVectorShuffle %v2int %154 %154 0 1
%156 = OpLoad %v4int %intGreen
%157 = OpVectorShuffle %v2int %156 %156 0 1
%153 = OpExtInst %v2int %1 SMax %155 %157
%158 = OpLoad %v4int %expectedB
%159 = OpVectorShuffle %v2int %158 %158 0 1
%160 = OpIEqual %v2bool %153 %159
%161 = OpAll %bool %160
OpBranch %152
%152 = OpLabel
%162 = OpPhi %bool %false %141 %161 %151
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpLoad %v4int %intValues
%167 = OpVectorShuffle %v3int %166 %166 0 1 2
%168 = OpLoad %v4int %intGreen
%169 = OpVectorShuffle %v3int %168 %168 0 1 2
%165 = OpExtInst %v3int %1 SMax %167 %169
%170 = OpLoad %v4int %expectedB
%171 = OpVectorShuffle %v3int %170 %170 0 1 2
%172 = OpIEqual %v3bool %165 %171
%173 = OpAll %bool %172
OpBranch %164
%164 = OpLabel
%174 = OpPhi %bool %false %152 %173 %163
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpLoad %v4int %intValues
%179 = OpLoad %v4int %intGreen
%177 = OpExtInst %v4int %1 SMax %178 %179
%180 = OpLoad %v4int %expectedB
%181 = OpIEqual %v4bool %177 %180
%182 = OpAll %bool %181
OpBranch %176
%176 = OpLabel
%183 = OpPhi %bool %false %164 %182 %175
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %v4int %expectedB
%187 = OpCompositeExtract %int %186 0
%188 = OpIEqual %bool %int_0 %187
OpBranch %185
%185 = OpLabel
%189 = OpPhi %bool %false %176 %188 %184
OpSelectionMerge %191 None
OpBranchConditional %189 %190 %191
%190 = OpLabel
%193 = OpLoad %v4int %expectedB
%194 = OpVectorShuffle %v2int %193 %193 0 1
%195 = OpIEqual %v2bool %192 %194
%196 = OpAll %bool %195
OpBranch %191
%191 = OpLabel
%197 = OpPhi %bool %false %185 %196 %190
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%201 = OpLoad %v4int %expectedB
%202 = OpVectorShuffle %v3int %201 %201 0 1 2
%203 = OpIEqual %v3bool %200 %202
%204 = OpAll %bool %203
OpBranch %199
%199 = OpLabel
%205 = OpPhi %bool %false %191 %204 %198
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%208 = OpLoad %v4int %expectedB
%209 = OpIEqual %v4bool %68 %208
%210 = OpAll %bool %209
OpBranch %207
%207 = OpLabel
%211 = OpPhi %bool %false %199 %210 %206
OpSelectionMerge %216 None
OpBranchConditional %211 %214 %215
%214 = OpLabel
%217 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%218 = OpLoad %v4float %217
OpStore %212 %218
OpBranch %216
%215 = OpLabel
%219 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%221 = OpLoad %v4float %219
OpStore %212 %221
OpBranch %216
%216 = OpLabel
%222 = OpLoad %v4float %212
OpReturnValue %222
OpFunctionEnd
