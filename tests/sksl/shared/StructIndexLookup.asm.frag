OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %InnerLUT "InnerLUT"
OpMemberName %InnerLUT 0 "values"
OpName %OuterLUT "OuterLUT"
OpMemberName %OuterLUT 0 "inner"
OpName %Root "Root"
OpMemberName %Root 0 "outer"
OpName %data "data"
OpName %expected "expected"
OpName %i "i"
OpName %j "j"
OpName %k "k"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpMemberDecorate %InnerLUT 0 Offset 0
OpDecorate %_arr_InnerLUT_int_3 ArrayStride 16
OpMemberDecorate %OuterLUT 0 Offset 0
OpMemberDecorate %OuterLUT 0 RelaxedPrecision
OpDecorate %_arr_OuterLUT_int_3 ArrayStride 48
OpMemberDecorate %Root 0 Offset 0
OpMemberDecorate %Root 0 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%InnerLUT = OpTypeStruct %v3float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%_arr_InnerLUT_int_3 = OpTypeArray %InnerLUT %int_3
%OuterLUT = OpTypeStruct %_arr_InnerLUT_int_3
%_arr_OuterLUT_int_3 = OpTypeArray %OuterLUT %int_3
%Root = OpTypeStruct %_arr_OuterLUT_int_3
%_ptr_Function_Root = OpTypePointer Function %Root
%float_1 = OpConstant %float 1
%float_10 = OpConstant %float 10
%float_100 = OpConstant %float 100
%39 = OpConstantComposite %v3float %float_1 %float_10 %float_100
%float_2 = OpConstant %float 2
%float_20 = OpConstant %float 20
%float_200 = OpConstant %float 200
%44 = OpConstantComposite %v3float %float_2 %float_20 %float_200
%float_3 = OpConstant %float 3
%float_30 = OpConstant %float 30
%float_300 = OpConstant %float 300
%49 = OpConstantComposite %v3float %float_3 %float_30 %float_300
%float_4 = OpConstant %float 4
%float_40 = OpConstant %float 40
%float_400 = OpConstant %float 400
%56 = OpConstantComposite %v3float %float_4 %float_40 %float_400
%float_5 = OpConstant %float 5
%float_50 = OpConstant %float 50
%float_500 = OpConstant %float 500
%61 = OpConstantComposite %v3float %float_5 %float_50 %float_500
%float_6 = OpConstant %float 6
%float_60 = OpConstant %float 60
%float_600 = OpConstant %float 600
%66 = OpConstantComposite %v3float %float_6 %float_60 %float_600
%float_7 = OpConstant %float 7
%float_70 = OpConstant %float 70
%float_700 = OpConstant %float 700
%73 = OpConstantComposite %v3float %float_7 %float_70 %float_700
%float_8 = OpConstant %float 8
%float_80 = OpConstant %float 80
%float_800 = OpConstant %float 800
%78 = OpConstantComposite %v3float %float_8 %float_80 %float_800
%float_9 = OpConstant %float 9
%float_90 = OpConstant %float 90
%float_900 = OpConstant %float 900
%83 = OpConstantComposite %v3float %float_9 %float_90 %float_900
%_ptr_Function_v3float = OpTypePointer Function %v3float
%91 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
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
%data = OpVariable %_ptr_Function_Root Function
%expected = OpVariable %_ptr_Function_v3float Function
%i = OpVariable %_ptr_Function_int Function
%j = OpVariable %_ptr_Function_int Function
%k = OpVariable %_ptr_Function_int Function
%40 = OpCompositeConstruct %InnerLUT %39
%45 = OpCompositeConstruct %InnerLUT %44
%50 = OpCompositeConstruct %InnerLUT %49
%51 = OpCompositeConstruct %_arr_InnerLUT_int_3 %40 %45 %50
%52 = OpCompositeConstruct %OuterLUT %51
%57 = OpCompositeConstruct %InnerLUT %56
%62 = OpCompositeConstruct %InnerLUT %61
%67 = OpCompositeConstruct %InnerLUT %66
%68 = OpCompositeConstruct %_arr_InnerLUT_int_3 %57 %62 %67
%69 = OpCompositeConstruct %OuterLUT %68
%74 = OpCompositeConstruct %InnerLUT %73
%79 = OpCompositeConstruct %InnerLUT %78
%84 = OpCompositeConstruct %InnerLUT %83
%85 = OpCompositeConstruct %_arr_InnerLUT_int_3 %74 %79 %84
%86 = OpCompositeConstruct %OuterLUT %85
%87 = OpCompositeConstruct %_arr_OuterLUT_int_3 %52 %69 %86
%88 = OpCompositeConstruct %Root %87
OpStore %data %88
OpStore %expected %91
OpStore %i %int_0
OpBranch %95
%95 = OpLabel
OpLoopMerge %99 %98 None
OpBranch %96
%96 = OpLabel
%100 = OpLoad %int %i
%101 = OpSLessThan %bool %100 %int_3
OpBranchConditional %101 %97 %99
%97 = OpLabel
OpStore %j %int_0
OpBranch %103
%103 = OpLabel
OpLoopMerge %107 %106 None
OpBranch %104
%104 = OpLabel
%108 = OpLoad %int %j
%109 = OpSLessThan %bool %108 %int_3
OpBranchConditional %109 %105 %107
%105 = OpLabel
%110 = OpLoad %v3float %expected
%111 = OpFAdd %v3float %110 %39
OpStore %expected %111
%112 = OpLoad %int %i
%113 = OpLoad %int %j
%114 = OpAccessChain %_ptr_Function_v3float %data %int_0 %112 %int_0 %113 %int_0
%115 = OpLoad %v3float %114
%116 = OpFUnordNotEqual %v3bool %115 %111
%118 = OpAny %bool %116
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%124 = OpLoad %v4float %121
OpReturnValue %124
%120 = OpLabel
OpStore %k %int_0
OpBranch %126
%126 = OpLabel
OpLoopMerge %130 %129 None
OpBranch %127
%127 = OpLabel
%131 = OpLoad %int %k
%132 = OpSLessThan %bool %131 %int_3
OpBranchConditional %132 %128 %130
%128 = OpLabel
%133 = OpLoad %int %i
%134 = OpLoad %int %j
%135 = OpAccessChain %_ptr_Function_v3float %data %int_0 %133 %int_0 %134 %int_0
%136 = OpLoad %v3float %135
%137 = OpLoad %int %k
%138 = OpVectorExtractDynamic %float %136 %137
%139 = OpLoad %v3float %expected
%140 = OpLoad %int %k
%141 = OpVectorExtractDynamic %float %139 %140
%142 = OpFUnordNotEqual %bool %138 %141
OpSelectionMerge %144 None
OpBranchConditional %142 %143 %144
%143 = OpLabel
%145 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%146 = OpLoad %v4float %145
OpReturnValue %146
%144 = OpLabel
OpBranch %129
%129 = OpLabel
%147 = OpLoad %int %k
%148 = OpIAdd %int %147 %int_1
OpStore %k %148
OpBranch %126
%130 = OpLabel
OpBranch %106
%106 = OpLabel
%149 = OpLoad %int %j
%150 = OpIAdd %int %149 %int_1
OpStore %j %150
OpBranch %103
%107 = OpLabel
OpBranch %98
%98 = OpLabel
%151 = OpLoad %int %i
%152 = OpIAdd %int %151 %int_1
OpStore %i %152
OpBranch %95
%99 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%154 = OpLoad %v4float %153
OpReturnValue %154
OpFunctionEnd
