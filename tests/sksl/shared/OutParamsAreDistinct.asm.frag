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
OpName %out_params_are_distinct_bhh "out_params_are_distinct_bhh"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %x RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%25 = OpTypeFunction %bool %_ptr_Function_float %_ptr_Function_float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%38 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%out_params_are_distinct_bhh = OpFunction %bool None %25
%26 = OpFunctionParameter %_ptr_Function_float
%27 = OpFunctionParameter %_ptr_Function_float
%28 = OpLabel
OpStore %26 %float_1
OpStore %27 %float_2
%32 = OpFOrdEqual %bool %float_1 %float_1
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%35 = OpLoad %float %27
%36 = OpFOrdEqual %bool %35 %float_2
OpBranch %34
%34 = OpLabel
%37 = OpPhi %bool %false %28 %36 %33
OpReturnValue %37
OpFunctionEnd
%main = OpFunction %v4float None %38
%39 = OpFunctionParameter %_ptr_Function_v2float
%40 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%42 = OpVariable %_ptr_Function_float Function
%43 = OpVariable %_ptr_Function_float Function
%47 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_0
%44 = OpFunctionCall %bool %out_params_are_distinct_bhh %42 %43
%45 = OpLoad %float %42
OpStore %x %45
%46 = OpLoad %float %43
OpStore %x %46
OpSelectionMerge %51 None
OpBranchConditional %44 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%56 = OpLoad %v4float %52
OpStore %47 %56
OpBranch %51
%50 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%59 = OpLoad %v4float %57
OpStore %47 %59
OpBranch %51
%51 = OpLabel
%60 = OpLoad %v4float %47
OpReturnValue %60
OpFunctionEnd
