OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%int_5 = OpConstant %int 5
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%a = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_int Function
%c = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_int Function
%56 = OpVariable %_ptr_Function_v4float Function
OpStore %a %int_0
OpStore %b %int_0
OpStore %c %int_0
OpStore %d %int_0
OpStore %a %int_1
OpStore %b %int_2
OpStore %c %int_5
%39 = OpLoad %int %a
%40 = OpIEqual %bool %39 %int_1
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%43 = OpLoad %int %b
%44 = OpIEqual %bool %43 %int_2
OpBranch %42
%42 = OpLabel
%45 = OpPhi %bool %false %27 %44 %41
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%48 = OpLoad %int %c
%49 = OpIEqual %bool %48 %int_5
OpBranch %47
%47 = OpLabel
%50 = OpPhi %bool %false %42 %49 %46
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%53 = OpLoad %int %d
%54 = OpIEqual %bool %53 %int_0
OpBranch %52
%52 = OpLabel
%55 = OpPhi %bool %false %47 %54 %51
OpSelectionMerge %60 None
OpBranchConditional %55 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%63 = OpLoad %v4float %61
OpStore %56 %63
OpBranch %60
%59 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%65 = OpLoad %v4float %64
OpStore %56 %65
OpBranch %60
%60 = OpLabel
%66 = OpLoad %v4float %56
OpReturnValue %66
OpFunctionEnd
