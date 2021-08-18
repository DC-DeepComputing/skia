OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %color "color"
OpName %counter "counter"
OpName %counter_0 "counter"
OpName %counter_1 "counter"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %color RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
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
%30 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
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
%color = OpVariable %_ptr_Function_v4float Function
%counter = OpVariable %_ptr_Function_int Function
%counter_0 = OpVariable %_ptr_Function_int Function
%counter_1 = OpVariable %_ptr_Function_int Function
OpStore %color %30
OpStore %counter %int_0
OpBranch %35
%35 = OpLabel
OpLoopMerge %39 %38 None
OpBranch %36
%36 = OpLabel
%40 = OpLoad %int %counter
%42 = OpSLessThan %bool %40 %int_10
OpBranchConditional %42 %37 %39
%37 = OpLabel
OpBranch %38
%38 = OpLabel
%44 = OpLoad %int %counter
%45 = OpIAdd %int %44 %int_1
OpStore %counter %45
OpBranch %35
%39 = OpLabel
OpStore %counter_0 %int_0
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
%52 = OpLoad %int %counter_0
%53 = OpSLessThan %bool %52 %int_10
OpBranchConditional %53 %49 %51
%49 = OpLabel
OpBranch %50
%50 = OpLabel
%54 = OpLoad %int %counter_0
%55 = OpIAdd %int %54 %int_1
OpStore %counter_0 %55
OpBranch %47
%51 = OpLabel
OpStore %counter_1 %int_0
OpBranch %57
%57 = OpLabel
OpLoopMerge %61 %60 None
OpBranch %58
%58 = OpLabel
%62 = OpLoad %int %counter_1
%63 = OpSLessThan %bool %62 %int_10
OpBranchConditional %63 %59 %61
%59 = OpLabel
OpBranch %60
%60 = OpLabel
%64 = OpLoad %int %counter_1
%65 = OpIAdd %int %64 %int_1
OpStore %counter_1 %65
OpBranch %57
%61 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%68 = OpLoad %v4float %66
%69 = OpCompositeExtract %float %68 0
%71 = OpFOrdEqual %bool %69 %float_1
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %74 %float_1
OpBranch %73
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%77 = OpLoad %v4float %76
%78 = OpCompositeExtract %float %77 0
%80 = OpFOrdEqual %bool %78 %float_2
OpSelectionMerge %83 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
OpBranch %83
%82 = OpLabel
%84 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %84 %float_1
OpBranch %83
%83 = OpLabel
OpBranch %86
%86 = OpLabel
OpLoopMerge %90 %89 None
OpBranch %87
%87 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%92 = OpLoad %v4float %91
%93 = OpCompositeExtract %float %92 0
%94 = OpFOrdEqual %bool %93 %float_2
OpBranchConditional %94 %88 %90
%88 = OpLabel
OpBranch %89
%89 = OpLabel
OpBranch %86
%90 = OpLabel
OpBranch %95
%95 = OpLabel
OpLoopMerge %99 %98 None
OpBranch %96
%96 = OpLabel
OpBranch %97
%97 = OpLabel
OpBranch %98
%98 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%101 = OpLoad %v4float %100
%102 = OpCompositeExtract %float %101 0
%103 = OpFOrdEqual %bool %102 %float_2
OpBranchConditional %103 %95 %99
%99 = OpLabel
%104 = OpLoad %v4float %color
OpReturnValue %104
OpFunctionEnd
