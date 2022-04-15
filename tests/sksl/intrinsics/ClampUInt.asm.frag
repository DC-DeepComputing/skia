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
OpDecorate %185 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
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
%74 = OpConstantComposite %v2uint %uint_100 %uint_100
%75 = OpConstantComposite %v2uint %uint_300 %uint_300
%v2bool = OpTypeVector %bool 2
%v3uint = OpTypeVector %uint 3
%86 = OpConstantComposite %v3uint %uint_100 %uint_100 %uint_100
%87 = OpConstantComposite %v3uint %uint_300 %uint_300 %uint_300
%v3bool = OpTypeVector %bool 3
%96 = OpConstantComposite %v4uint %uint_100 %uint_100 %uint_100 %uint_100
%97 = OpConstantComposite %v4uint %uint_300 %uint_300 %uint_300 %uint_300
%v4bool = OpTypeVector %bool 4
%108 = OpConstantComposite %v2uint %uint_100 %uint_200
%115 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
%134 = OpConstantComposite %v2uint %uint_100 %uint_0
%135 = OpConstantComposite %v2uint %uint_300 %uint_400
%144 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
%145 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
%168 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
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
%178 = OpVariable %_ptr_Function_v4float Function
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
%67 = OpExtInst %uint %1 UClamp %41 %uint_100 %uint_300
%68 = OpIEqual %bool %67 %uint_100
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpVectorShuffle %v2uint %48 %48 0 1
%71 = OpExtInst %v2uint %1 UClamp %72 %74 %75
%76 = OpVectorShuffle %v2uint %54 %54 0 1
%78 = OpIEqual %v2bool %71 %76
%79 = OpAll %bool %78
OpBranch %70
%70 = OpLabel
%80 = OpPhi %bool %false %25 %79 %69
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpVectorShuffle %v3uint %48 %48 0 1 2
%83 = OpExtInst %v3uint %1 UClamp %84 %86 %87
%88 = OpVectorShuffle %v3uint %54 %54 0 1 2
%90 = OpIEqual %v3bool %83 %88
%91 = OpAll %bool %90
OpBranch %82
%82 = OpLabel
%92 = OpPhi %bool %false %70 %91 %81
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpExtInst %v4uint %1 UClamp %48 %96 %97
%99 = OpIEqual %v4bool %95 %54
%100 = OpAll %bool %99
OpBranch %94
%94 = OpLabel
%101 = OpPhi %bool %false %82 %100 %93
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpIEqual %bool %uint_100 %uint_100
OpBranch %103
%103 = OpLabel
%105 = OpPhi %bool %false %94 %104 %102
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%109 = OpVectorShuffle %v2uint %54 %54 0 1
%110 = OpIEqual %v2bool %108 %109
%111 = OpAll %bool %110
OpBranch %107
%107 = OpLabel
%112 = OpPhi %bool %false %103 %111 %106
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpVectorShuffle %v3uint %54 %54 0 1 2
%117 = OpIEqual %v3bool %115 %116
%118 = OpAll %bool %117
OpBranch %114
%114 = OpLabel
%119 = OpPhi %bool %false %107 %118 %113
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpIEqual %v4bool %54 %54
%123 = OpAll %bool %122
OpBranch %121
%121 = OpLabel
%124 = OpPhi %bool %false %114 %123 %120
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpExtInst %uint %1 UClamp %41 %uint_100 %uint_300
%128 = OpIEqual %bool %127 %uint_100
OpBranch %126
%126 = OpLabel
%129 = OpPhi %bool %false %121 %128 %125
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpVectorShuffle %v2uint %48 %48 0 1
%132 = OpExtInst %v2uint %1 UClamp %133 %134 %135
%136 = OpVectorShuffle %v2uint %61 %61 0 1
%137 = OpIEqual %v2bool %132 %136
%138 = OpAll %bool %137
OpBranch %131
%131 = OpLabel
%139 = OpPhi %bool %false %126 %138 %130
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%143 = OpVectorShuffle %v3uint %48 %48 0 1 2
%142 = OpExtInst %v3uint %1 UClamp %143 %144 %145
%146 = OpVectorShuffle %v3uint %61 %61 0 1 2
%147 = OpIEqual %v3bool %142 %146
%148 = OpAll %bool %147
OpBranch %141
%141 = OpLabel
%149 = OpPhi %bool %false %131 %148 %140
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpExtInst %v4uint %1 UClamp %48 %57 %65
%153 = OpIEqual %v4bool %152 %61
%154 = OpAll %bool %153
OpBranch %151
%151 = OpLabel
%155 = OpPhi %bool %false %141 %154 %150
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpIEqual %bool %uint_100 %uint_100
OpBranch %157
%157 = OpLabel
%159 = OpPhi %bool %false %151 %158 %156
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpVectorShuffle %v2uint %61 %61 0 1
%163 = OpIEqual %v2bool %108 %162
%164 = OpAll %bool %163
OpBranch %161
%161 = OpLabel
%165 = OpPhi %bool %false %157 %164 %160
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%169 = OpVectorShuffle %v3uint %61 %61 0 1 2
%170 = OpIEqual %v3bool %168 %169
%171 = OpAll %bool %170
OpBranch %167
%167 = OpLabel
%172 = OpPhi %bool %false %161 %171 %166
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpIEqual %v4bool %61 %61
%176 = OpAll %bool %175
OpBranch %174
%174 = OpLabel
%177 = OpPhi %bool %false %167 %176 %173
OpSelectionMerge %182 None
OpBranchConditional %177 %180 %181
%180 = OpLabel
%183 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%185 = OpLoad %v4float %183
OpStore %178 %185
OpBranch %182
%181 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%188 = OpLoad %v4float %186
OpStore %178 %188
OpBranch %182
%182 = OpLabel
%189 = OpLoad %v4float %178
OpReturnValue %189
OpFunctionEnd
