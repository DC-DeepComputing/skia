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
OpName %color "color"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_5 = OpConstant %float 0.5
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%_ptr_Function_float = OpTypePointer Function %float
%int_3 = OpConstant %int 3
%int_1 = OpConstant %int 1
%float_4 = OpConstant %float 4
%v3float = OpTypeVector %float 3
%48 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%56 = OpConstantComposite %v4float %float_0_25 %float_0 %float_0 %float_0_75
%float_1 = OpConstant %float 1
%73 = OpConstantComposite %v4float %float_1 %float_1 %float_0_25 %float_1
%v4bool = OpTypeVector %bool 4
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
%color = OpVariable %_ptr_Function_v4float Function
%65 = OpVariable %_ptr_Function_float Function
%77 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%34 = OpVectorTimesScalar %v4float %32 %float_0_5
OpStore %color %34
%37 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %37 %float_2
%40 = OpAccessChain %_ptr_Function_float %color %int_1
%42 = OpLoad %float %40
%44 = OpFMul %float %42 %float_4
OpStore %40 %44
%45 = OpLoad %v4float %color
%46 = OpVectorShuffle %v3float %45 %45 1 2 3
%49 = OpFMul %v3float %46 %48
%50 = OpLoad %v4float %color
%51 = OpVectorShuffle %v4float %50 %49 0 4 5 6
OpStore %color %51
%52 = OpLoad %v4float %color
%53 = OpVectorShuffle %v4float %52 %52 2 1 3 0
%57 = OpFAdd %v4float %53 %56
%58 = OpLoad %v4float %color
%59 = OpVectorShuffle %v4float %58 %57 7 5 4 6
OpStore %color %59
%60 = OpAccessChain %_ptr_Function_float %color %int_0
%61 = OpLoad %float %60
%62 = OpCompositeExtract %float %59 3
%64 = OpFOrdLessThanEqual %bool %62 %float_1
OpSelectionMerge %68 None
OpBranchConditional %64 %66 %67
%66 = OpLabel
%69 = OpCompositeExtract %float %59 2
OpStore %65 %69
OpBranch %68
%67 = OpLabel
OpStore %65 %float_0
OpBranch %68
%68 = OpLabel
%70 = OpLoad %float %65
%71 = OpFAdd %float %61 %70
OpStore %60 %71
%72 = OpLoad %v4float %color
%74 = OpFOrdEqual %v4bool %72 %73
%76 = OpAll %bool %74
OpSelectionMerge %80 None
OpBranchConditional %76 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%82 = OpLoad %v4float %81
OpStore %77 %82
OpBranch %80
%79 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%84 = OpLoad %v4float %83
OpStore %77 %84
OpBranch %80
%80 = OpLabel
%85 = OpLoad %v4float %77
OpReturnValue %85
OpFunctionEnd
