OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_switchValue "_0_switchValue"
OpName %_2_switchFallthrough "_2_switchFallthrough"
OpName %_3_switchLoop "_3_switchLoop"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%int_1 = OpConstant %int 1
%true = OpConstantTrue %bool
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
%_0_switchValue = OpVariable %_ptr_Function_int Function
%_2_switchFallthrough = OpVariable %_ptr_Function_bool Function
%_3_switchLoop = OpVariable %_ptr_Function_int Function
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %29
%33 = OpCompositeExtract %float %32 1
%34 = OpConvertFToS %int %33
OpStore %_0_switchValue %34
OpStore %_2_switchFallthrough %false
OpStore %_3_switchLoop %int_0
OpBranch %39
%39 = OpLabel
OpLoopMerge %43 %42 None
OpBranch %40
%40 = OpLabel
%44 = OpLoad %int %_3_switchLoop
%46 = OpSLessThan %bool %44 %int_1
OpBranchConditional %46 %41 %43
%41 = OpLabel
%47 = OpLoad %int %_0_switchValue
%48 = OpIEqual %bool %47 %int_0
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%52 = OpLoad %v4float %51
OpReturnValue %52
%50 = OpLabel
%54 = OpLoad %bool %_2_switchFallthrough
OpSelectionMerge %56 None
OpBranchConditional %54 %56 %55
%55 = OpLabel
%57 = OpLoad %int %_0_switchValue
%58 = OpIEqual %bool %57 %int_1
OpBranch %56
%56 = OpLabel
%59 = OpPhi %bool %true %50 %58 %55
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
OpReturnValue %63
%61 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%65 = OpLoad %v4float %64
OpReturnValue %65
%42 = OpLabel
%66 = OpLoad %int %_3_switchLoop
%67 = OpIAdd %int %66 %int_1
OpStore %_3_switchLoop %67
OpBranch %39
%43 = OpLabel
OpUnreachable
OpFunctionEnd
