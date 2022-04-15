OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %main "main"
OpName %i "i"
OpName %i_0 "i"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_5 = OpConstant %float 5
%float_0_75 = OpConstant %float 0.75
%27 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%_ptr_Function_int = OpTypePointer Function %int
%int_10 = OpConstant %int 10
%float_0_5 = OpConstant %float 0.5
%int_1 = OpConstant %int 1
%float_0_25 = OpConstant %float 0.25
%51 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%int_2 = OpConstant %int 2
%int_100 = OpConstant %int 100
%main = OpFunction %void None %14
%15 = OpLabel
%i = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
%16 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%20 = OpLoad %float %16
%22 = OpFOrdGreaterThan %bool %20 %float_5
OpSelectionMerge %25 None
OpBranchConditional %22 %23 %24
%23 = OpLabel
OpStore %sk_FragColor %27
OpBranch %25
%24 = OpLabel
OpKill
%25 = OpLabel
OpStore %i %int_0
OpBranch %30
%30 = OpLabel
OpLoopMerge %34 %33 None
OpBranch %31
%31 = OpLabel
%35 = OpLoad %int %i
%37 = OpSLessThan %bool %35 %int_10
OpBranchConditional %37 %32 %34
%32 = OpLabel
%38 = OpLoad %v4float %sk_FragColor
%40 = OpVectorTimesScalar %v4float %38 %float_0_5
OpStore %sk_FragColor %40
%41 = OpLoad %int %i
%43 = OpIAdd %int %41 %int_1
OpStore %i %43
OpBranch %33
%33 = OpLabel
OpBranch %30
%34 = OpLabel
OpBranch %44
%44 = OpLabel
OpLoopMerge %48 %47 None
OpBranch %45
%45 = OpLabel
%49 = OpLoad %v4float %sk_FragColor
%52 = OpFAdd %v4float %49 %51
OpStore %sk_FragColor %52
OpBranch %46
%46 = OpLabel
OpBranch %47
%47 = OpLabel
%53 = OpCompositeExtract %float %52 0
%54 = OpFOrdLessThan %bool %53 %float_0_75
OpBranchConditional %54 %44 %48
%48 = OpLabel
OpStore %i_0 %int_0
OpBranch %56
%56 = OpLabel
OpLoopMerge %60 %59 None
OpBranch %57
%57 = OpLabel
%61 = OpLoad %int %i_0
%62 = OpSLessThan %bool %61 %int_10
OpBranchConditional %62 %58 %60
%58 = OpLabel
%63 = OpLoad %int %i_0
%65 = OpSMod %int %63 %int_2
%66 = OpIEqual %bool %65 %int_1
OpSelectionMerge %69 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
OpBranch %60
%68 = OpLabel
%70 = OpLoad %int %i_0
%72 = OpSGreaterThan %bool %70 %int_100
OpSelectionMerge %75 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
OpReturn
%74 = OpLabel
OpBranch %59
%75 = OpLabel
OpBranch %69
%69 = OpLabel
OpBranch %59
%59 = OpLabel
%76 = OpLoad %int %i_0
%77 = OpIAdd %int %76 %int_1
OpStore %i_0 %77
OpBranch %56
%60 = OpLabel
OpReturn
OpFunctionEnd
