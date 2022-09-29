OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_src_over_h4h4h4 "blend_src_over_h4h4h4"
OpName %blend_darken_h4hh4h4 "blend_darken_h4hh4h4"
OpName %a "a"
OpName %b "b"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v4float = OpTypePointer Function %v4float
%16 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%29 = OpTypeFunction %v4float %_ptr_Function_float %_ptr_Function_v4float %_ptr_Function_v4float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%void = OpTypeVoid
%63 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_src_over_h4h4h4 = OpFunction %v4float None %16
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpLabel
%20 = OpLoad %v4float %17
%22 = OpLoad %v4float %17
%23 = OpCompositeExtract %float %22 3
%24 = OpFSub %float %float_1 %23
%25 = OpLoad %v4float %18
%26 = OpVectorTimesScalar %v4float %25 %24
%27 = OpFAdd %v4float %20 %26
OpReturnValue %27
OpFunctionEnd
%blend_darken_h4hh4h4 = OpFunction %v4float None %29
%30 = OpFunctionParameter %_ptr_Function_float
%31 = OpFunctionParameter %_ptr_Function_v4float
%32 = OpFunctionParameter %_ptr_Function_v4float
%33 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%36 = OpVariable %_ptr_Function_v4float Function
%38 = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v3float Function
%35 = OpLoad %v4float %31
OpStore %36 %35
%37 = OpLoad %v4float %32
OpStore %38 %37
%39 = OpFunctionCall %v4float %blend_src_over_h4h4h4 %36 %38
OpStore %a %39
%43 = OpLoad %v4float %32
%44 = OpCompositeExtract %float %43 3
%45 = OpFSub %float %float_1 %44
%46 = OpLoad %v4float %31
%47 = OpVectorShuffle %v3float %46 %46 0 1 2
%48 = OpVectorTimesScalar %v3float %47 %45
%49 = OpLoad %v4float %32
%50 = OpVectorShuffle %v3float %49 %49 0 1 2
%51 = OpFAdd %v3float %48 %50
OpStore %b %51
%52 = OpLoad %float %30
%54 = OpVectorShuffle %v3float %39 %39 0 1 2
%55 = OpLoad %float %30
%56 = OpVectorTimesScalar %v3float %54 %55
%57 = OpLoad %float %30
%58 = OpVectorTimesScalar %v3float %51 %57
%53 = OpExtInst %v3float %1 FMin %56 %58
%59 = OpVectorTimesScalar %v3float %53 %52
%60 = OpLoad %v4float %a
%61 = OpVectorShuffle %v4float %60 %59 4 5 6 3
OpStore %a %61
OpReturnValue %61
OpFunctionEnd
%main = OpFunction %void None %63
%64 = OpLabel
%65 = OpVariable %_ptr_Function_float Function
%71 = OpVariable %_ptr_Function_v4float Function
%75 = OpVariable %_ptr_Function_v4float Function
OpStore %65 %float_1
%66 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%70 = OpLoad %v4float %66
OpStore %71 %70
%72 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%74 = OpLoad %v4float %72
OpStore %75 %74
%76 = OpFunctionCall %v4float %blend_darken_h4hh4h4 %65 %71 %75
OpStore %sk_FragColor %76
OpReturn
OpFunctionEnd
