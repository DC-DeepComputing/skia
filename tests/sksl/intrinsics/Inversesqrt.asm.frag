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
OpName %negativeVal "negativeVal"
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
OpDecorate %negativeVal RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_n4 = OpConstant %float -4
%float_n16 = OpConstant %float -16
%float_n64 = OpConstant %float -64
%34 = OpConstantComposite %v4float %float_n1 %float_n4 %float_n16 %float_n64
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%float_0_5 = OpConstant %float 0.5
%97 = OpConstantComposite %v2float %float_1 %float_0_5
%float_0_25 = OpConstant %float 0.25
%107 = OpConstantComposite %v3float %float_1 %float_0_5 %float_0_25
%float_0_125 = OpConstant %float 0.125
%117 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0_25 %float_0_125
%134 = OpConstantComposite %v2float %float_n1 %float_n4
%144 = OpConstantComposite %v3float %float_n1 %float_n4 %float_n16
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
%negativeVal = OpVariable %_ptr_Function_v4float Function
%160 = OpVariable %_ptr_Function_v4float Function
OpStore %negativeVal %34
%37 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%36 = OpExtInst %float %1 InverseSqrt %42
%43 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%45 = OpLoad %v4float %43
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %36 %46
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
%50 = OpExtInst %v2float %1 InverseSqrt %53
%54 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%55 = OpLoad %v4float %54
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
%63 = OpExtInst %v3float %1 InverseSqrt %66
%68 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%69 = OpLoad %v4float %68
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
%77 = OpExtInst %v4float %1 InverseSqrt %79
%80 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%81 = OpLoad %v4float %80
%82 = OpFOrdEqual %v4bool %77 %81
%84 = OpAll %bool %82
OpBranch %76
%76 = OpLabel
%85 = OpPhi %bool %false %62 %84 %75
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%90 = OpLoad %v4float %89
%91 = OpCompositeExtract %float %90 0
%92 = OpFOrdEqual %bool %float_1 %91
OpBranch %87
%87 = OpLabel
%93 = OpPhi %bool %false %76 %92 %86
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%99 = OpLoad %v4float %98
%100 = OpVectorShuffle %v2float %99 %99 0 1
%101 = OpFOrdEqual %v2bool %97 %100
%102 = OpAll %bool %101
OpBranch %95
%95 = OpLabel
%103 = OpPhi %bool %false %87 %102 %94
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v3float %109 %109 0 1 2
%111 = OpFOrdEqual %v3bool %107 %110
%112 = OpAll %bool %111
OpBranch %105
%105 = OpLabel
%113 = OpPhi %bool %false %95 %112 %104
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%119 = OpLoad %v4float %118
%120 = OpFOrdEqual %v4bool %117 %119
%121 = OpAll %bool %120
OpBranch %115
%115 = OpLabel
%122 = OpPhi %bool %false %105 %121 %114
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpExtInst %float %1 InverseSqrt %float_n1
%126 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%127 = OpLoad %v4float %126
%128 = OpCompositeExtract %float %127 0
%129 = OpFOrdEqual %bool %125 %128
OpBranch %124
%124 = OpLabel
%130 = OpPhi %bool %false %115 %129 %123
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpExtInst %v2float %1 InverseSqrt %134
%135 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%136 = OpLoad %v4float %135
%137 = OpVectorShuffle %v2float %136 %136 0 1
%138 = OpFOrdEqual %v2bool %133 %137
%139 = OpAll %bool %138
OpBranch %132
%132 = OpLabel
%140 = OpPhi %bool %false %124 %139 %131
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpExtInst %v3float %1 InverseSqrt %144
%145 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%146 = OpLoad %v4float %145
%147 = OpVectorShuffle %v3float %146 %146 0 1 2
%148 = OpFOrdEqual %v3bool %143 %147
%149 = OpAll %bool %148
OpBranch %142
%142 = OpLabel
%150 = OpPhi %bool %false %132 %149 %141
OpSelectionMerge %152 None
OpBranchConditional %150 %151 %152
%151 = OpLabel
%154 = OpLoad %v4float %negativeVal
%153 = OpExtInst %v4float %1 InverseSqrt %154
%155 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%156 = OpLoad %v4float %155
%157 = OpFOrdEqual %v4bool %153 %156
%158 = OpAll %bool %157
OpBranch %152
%152 = OpLabel
%159 = OpPhi %bool %false %142 %158 %151
OpSelectionMerge %163 None
OpBranchConditional %159 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%166 = OpLoad %v4float %164
OpStore %160 %166
OpBranch %163
%162 = OpLabel
%167 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%169 = OpLoad %v4float %167
OpStore %160 %169
OpBranch %163
%163 = OpLabel
%170 = OpLoad %v4float %160
OpReturnValue %170
OpFunctionEnd
