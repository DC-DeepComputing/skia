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
OpName %uintValues "uintValues"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
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
%uint = OpTypeInt 32 0
%v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%float_200 = OpConstant %float 200
%38 = OpConstantComposite %v4float %float_200 %float_200 %float_200 %float_200
%uint_100 = OpConstant %uint 100
%uint_200 = OpConstant %uint 200
%uint_275 = OpConstant %uint 275
%uint_300 = OpConstant %uint 300
%54 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_275 %uint_300
%uint_0 = OpConstant %uint 0
%57 = OpConstantComposite %v4uint %uint_100 %uint_0 %uint_0 %uint_300
%uint_250 = OpConstant %uint 250
%uint_425 = OpConstant %uint 425
%61 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_250 %uint_425
%uint_400 = OpConstant %uint 400
%uint_500 = OpConstant %uint 500
%65 = OpConstantComposite %v4uint %uint_300 %uint_400 %uint_250 %uint_500
%false = OpConstantFalse %bool
%v2uint = OpTypeVector %uint 2
%76 = OpConstantComposite %v2uint %uint_100 %uint_100
%77 = OpConstantComposite %v2uint %uint_300 %uint_300
%v2bool = OpTypeVector %bool 2
%v3uint = OpTypeVector %uint 3
%90 = OpConstantComposite %v3uint %uint_100 %uint_100 %uint_100
%91 = OpConstantComposite %v3uint %uint_300 %uint_300 %uint_300
%v3bool = OpTypeVector %bool 3
%102 = OpConstantComposite %v4uint %uint_100 %uint_100 %uint_100 %uint_100
%103 = OpConstantComposite %v4uint %uint_300 %uint_300 %uint_300 %uint_300
%v4bool = OpTypeVector %bool 4
%117 = OpConstantComposite %v2uint %uint_100 %uint_200
%125 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
%151 = OpConstantComposite %v2uint %uint_100 %uint_0
%152 = OpConstantComposite %v2uint %uint_300 %uint_400
%163 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
%164 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
%195 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
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
%uintValues = OpVariable %_ptr_Function_v4uint Function
%expectedA = OpVariable %_ptr_Function_v4uint Function
%clampLow = OpVariable %_ptr_Function_v4uint Function
%expectedB = OpVariable %_ptr_Function_v4uint Function
%clampHigh = OpVariable %_ptr_Function_v4uint Function
%207 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %30
%36 = OpVectorTimesScalar %v4float %34 %float_100
%39 = OpFAdd %v4float %36 %38
%40 = OpCompositeExtract %float %39 0
%41 = OpConvertFToU %uint %40
%42 = OpCompositeExtract %float %39 1
%43 = OpConvertFToU %uint %42
%44 = OpCompositeExtract %float %39 2
%45 = OpConvertFToU %uint %44
%46 = OpCompositeExtract %float %39 3
%47 = OpConvertFToU %uint %46
%48 = OpCompositeConstruct %v4uint %41 %43 %45 %47
OpStore %uintValues %48
OpStore %expectedA %54
OpStore %clampLow %57
OpStore %expectedB %61
OpStore %clampHigh %65
%68 = OpCompositeExtract %uint %48 0
%67 = OpExtInst %uint %1 UClamp %68 %uint_100 %uint_300
%69 = OpIEqual %bool %67 %uint_100
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%73 = OpLoad %v4uint %uintValues
%74 = OpVectorShuffle %v2uint %73 %73 0 1
%72 = OpExtInst %v2uint %1 UClamp %74 %76 %77
%78 = OpLoad %v4uint %expectedA
%79 = OpVectorShuffle %v2uint %78 %78 0 1
%80 = OpIEqual %v2bool %72 %79
%82 = OpAll %bool %80
OpBranch %71
%71 = OpLabel
%83 = OpPhi %bool %false %25 %82 %70
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpLoad %v4uint %uintValues
%88 = OpVectorShuffle %v3uint %87 %87 0 1 2
%86 = OpExtInst %v3uint %1 UClamp %88 %90 %91
%92 = OpLoad %v4uint %expectedA
%93 = OpVectorShuffle %v3uint %92 %92 0 1 2
%94 = OpIEqual %v3bool %86 %93
%96 = OpAll %bool %94
OpBranch %85
%85 = OpLabel
%97 = OpPhi %bool %false %71 %96 %84
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpLoad %v4uint %uintValues
%100 = OpExtInst %v4uint %1 UClamp %101 %102 %103
%104 = OpLoad %v4uint %expectedA
%105 = OpIEqual %v4bool %100 %104
%107 = OpAll %bool %105
OpBranch %99
%99 = OpLabel
%108 = OpPhi %bool %false %85 %107 %98
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %v4uint %expectedA
%112 = OpCompositeExtract %uint %111 0
%113 = OpIEqual %bool %uint_100 %112
OpBranch %110
%110 = OpLabel
%114 = OpPhi %bool %false %99 %113 %109
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %v4uint %expectedA
%119 = OpVectorShuffle %v2uint %118 %118 0 1
%120 = OpIEqual %v2bool %117 %119
%121 = OpAll %bool %120
OpBranch %116
%116 = OpLabel
%122 = OpPhi %bool %false %110 %121 %115
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpLoad %v4uint %expectedA
%127 = OpVectorShuffle %v3uint %126 %126 0 1 2
%128 = OpIEqual %v3bool %125 %127
%129 = OpAll %bool %128
OpBranch %124
%124 = OpLabel
%130 = OpPhi %bool %false %116 %129 %123
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %v4uint %expectedA
%134 = OpIEqual %v4bool %54 %133
%135 = OpAll %bool %134
OpBranch %132
%132 = OpLabel
%136 = OpPhi %bool %false %124 %135 %131
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%140 = OpLoad %v4uint %uintValues
%141 = OpCompositeExtract %uint %140 0
%139 = OpExtInst %uint %1 UClamp %141 %uint_100 %uint_300
%142 = OpLoad %v4uint %expectedB
%143 = OpCompositeExtract %uint %142 0
%144 = OpIEqual %bool %139 %143
OpBranch %138
%138 = OpLabel
%145 = OpPhi %bool %false %132 %144 %137
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%149 = OpLoad %v4uint %uintValues
%150 = OpVectorShuffle %v2uint %149 %149 0 1
%148 = OpExtInst %v2uint %1 UClamp %150 %151 %152
%153 = OpLoad %v4uint %expectedB
%154 = OpVectorShuffle %v2uint %153 %153 0 1
%155 = OpIEqual %v2bool %148 %154
%156 = OpAll %bool %155
OpBranch %147
%147 = OpLabel
%157 = OpPhi %bool %false %138 %156 %146
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%161 = OpLoad %v4uint %uintValues
%162 = OpVectorShuffle %v3uint %161 %161 0 1 2
%160 = OpExtInst %v3uint %1 UClamp %162 %163 %164
%165 = OpLoad %v4uint %expectedB
%166 = OpVectorShuffle %v3uint %165 %165 0 1 2
%167 = OpIEqual %v3bool %160 %166
%168 = OpAll %bool %167
OpBranch %159
%159 = OpLabel
%169 = OpPhi %bool %false %147 %168 %158
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%173 = OpLoad %v4uint %uintValues
%174 = OpLoad %v4uint %clampLow
%175 = OpLoad %v4uint %clampHigh
%172 = OpExtInst %v4uint %1 UClamp %173 %174 %175
%176 = OpLoad %v4uint %expectedB
%177 = OpIEqual %v4bool %172 %176
%178 = OpAll %bool %177
OpBranch %171
%171 = OpLabel
%179 = OpPhi %bool %false %159 %178 %170
OpSelectionMerge %181 None
OpBranchConditional %179 %180 %181
%180 = OpLabel
%182 = OpLoad %v4uint %expectedB
%183 = OpCompositeExtract %uint %182 0
%184 = OpIEqual %bool %uint_100 %183
OpBranch %181
%181 = OpLabel
%185 = OpPhi %bool %false %171 %184 %180
OpSelectionMerge %187 None
OpBranchConditional %185 %186 %187
%186 = OpLabel
%188 = OpLoad %v4uint %expectedB
%189 = OpVectorShuffle %v2uint %188 %188 0 1
%190 = OpIEqual %v2bool %117 %189
%191 = OpAll %bool %190
OpBranch %187
%187 = OpLabel
%192 = OpPhi %bool %false %181 %191 %186
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%196 = OpLoad %v4uint %expectedB
%197 = OpVectorShuffle %v3uint %196 %196 0 1 2
%198 = OpIEqual %v3bool %195 %197
%199 = OpAll %bool %198
OpBranch %194
%194 = OpLabel
%200 = OpPhi %bool %false %187 %199 %193
OpSelectionMerge %202 None
OpBranchConditional %200 %201 %202
%201 = OpLabel
%203 = OpLoad %v4uint %expectedB
%204 = OpIEqual %v4bool %61 %203
%205 = OpAll %bool %204
OpBranch %202
%202 = OpLabel
%206 = OpPhi %bool %false %194 %205 %201
OpSelectionMerge %211 None
OpBranchConditional %206 %209 %210
%209 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%214 = OpLoad %v4float %212
OpStore %207 %214
OpBranch %211
%210 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%217 = OpLoad %v4float %215
OpStore %207 %217
OpBranch %211
%211 = OpLabel
%218 = OpLoad %v4float %207
OpReturnValue %218
OpFunctionEnd
