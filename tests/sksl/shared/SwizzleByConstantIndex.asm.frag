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
OpName %_0_v "_0_v"
OpName %_1_x "_1_x"
OpName %_2_y "_2_y"
OpName %_3_z "_3_z"
OpName %_4_w "_4_w"
OpName %a "a"
OpName %_9_x "_9_x"
OpName %_10_y "_10_y"
OpName %_11_z "_11_z"
OpName %_12_w "_12_w"
OpName %b "b"
OpName %_13_v "_13_v"
OpName %_14_x "_14_x"
OpName %_15_y "_15_y"
OpName %_16_z "_16_z"
OpName %_17_w "_17_w"
OpName %c "c"
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
OpDecorate %_0_v RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %_1_x RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %_2_y RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %_3_z RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %_4_w RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %_9_x RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %_10_y RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %_11_z RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %_12_w RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %_13_v RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %_14_x RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %_15_y RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %_16_z RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %_17_w RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%80 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%104 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
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
%_0_v = OpVariable %_ptr_Function_v4float Function
%_1_x = OpVariable %_ptr_Function_float Function
%_2_y = OpVariable %_ptr_Function_float Function
%_3_z = OpVariable %_ptr_Function_float Function
%_4_w = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_v4float Function
%_9_x = OpVariable %_ptr_Function_float Function
%_10_y = OpVariable %_ptr_Function_float Function
%_11_z = OpVariable %_ptr_Function_float Function
%_12_w = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_v4float Function
%_13_v = OpVariable %_ptr_Function_v4float Function
%_14_x = OpVariable %_ptr_Function_float Function
%_15_y = OpVariable %_ptr_Function_float Function
%_16_z = OpVariable %_ptr_Function_float Function
%_17_w = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_v4float Function
%120 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
OpStore %_0_v %34
%37 = OpLoad %v4float %_0_v
%38 = OpCompositeExtract %float %37 0
OpStore %_1_x %38
%40 = OpLoad %v4float %_0_v
%41 = OpCompositeExtract %float %40 1
OpStore %_2_y %41
%43 = OpLoad %v4float %_0_v
%44 = OpCompositeExtract %float %43 2
OpStore %_3_z %44
%46 = OpLoad %v4float %_0_v
%47 = OpCompositeExtract %float %46 3
OpStore %_4_w %47
%49 = OpLoad %float %_1_x
%50 = OpLoad %float %_2_y
%51 = OpLoad %float %_3_z
%52 = OpLoad %float %_4_w
%53 = OpCompositeConstruct %v4float %49 %50 %51 %52
OpStore %a %53
%55 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 0
OpStore %_9_x %57
%59 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%60 = OpLoad %v4float %59
%61 = OpCompositeExtract %float %60 1
OpStore %_10_y %61
%63 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%64 = OpLoad %v4float %63
%65 = OpCompositeExtract %float %64 2
OpStore %_11_z %65
%67 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%68 = OpLoad %v4float %67
%69 = OpCompositeExtract %float %68 3
OpStore %_12_w %69
%71 = OpLoad %float %_9_x
%72 = OpLoad %float %_10_y
%73 = OpLoad %float %_11_z
%74 = OpLoad %float %_12_w
%75 = OpCompositeConstruct %v4float %71 %72 %73 %74
OpStore %b %75
OpStore %_13_v %80
%82 = OpLoad %v4float %_13_v
%83 = OpCompositeExtract %float %82 0
OpStore %_14_x %83
%85 = OpLoad %v4float %_13_v
%86 = OpCompositeExtract %float %85 1
OpStore %_15_y %86
%88 = OpLoad %v4float %_13_v
%89 = OpCompositeExtract %float %88 2
OpStore %_16_z %89
%91 = OpLoad %v4float %_13_v
%92 = OpCompositeExtract %float %91 3
OpStore %_17_w %92
%94 = OpLoad %float %_14_x
%95 = OpLoad %float %_15_y
%96 = OpLoad %float %_16_z
%97 = OpLoad %float %_17_w
%98 = OpCompositeConstruct %v4float %94 %95 %96 %97
OpStore %c %98
%100 = OpLoad %v4float %a
%105 = OpFOrdEqual %v4bool %100 %104
%107 = OpAll %bool %105
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpLoad %v4float %b
%111 = OpFOrdEqual %v4bool %110 %104
%112 = OpAll %bool %111
OpBranch %109
%109 = OpLabel
%113 = OpPhi %bool %false %27 %112 %108
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %v4float %c
%117 = OpFOrdEqual %v4bool %116 %80
%118 = OpAll %bool %117
OpBranch %115
%115 = OpLabel
%119 = OpPhi %bool %false %109 %118 %114
OpSelectionMerge %123 None
OpBranchConditional %119 %121 %122
%121 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%126 = OpLoad %v4float %124
OpStore %120 %126
OpBranch %123
%122 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%129 = OpLoad %v4float %127
OpStore %120 %129
OpBranch %123
%123 = OpLabel
%130 = OpLoad %v4float %120
OpReturnValue %130
OpFunctionEnd
