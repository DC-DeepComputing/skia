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
OpName %ok "ok"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
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
%ok = OpVariable %_ptr_Function_bool Function
%55 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%32 = OpLoad %bool %ok
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 1
%42 = OpFOrdEqual %bool %40 %float_1
%43 = OpSelect %bool %42 %true %false
OpBranch %34
%34 = OpLabel
%44 = OpPhi %bool %false %27 %43 %33
OpStore %ok %44
%45 = OpLoad %bool %ok
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%49 = OpLoad %v4float %48
%50 = OpCompositeExtract %float %49 0
%51 = OpFOrdEqual %bool %50 %float_1
%52 = OpSelect %bool %51 %false %true
OpBranch %47
%47 = OpLabel
%53 = OpPhi %bool %false %34 %52 %46
OpStore %ok %53
%54 = OpLoad %bool %ok
OpSelectionMerge %59 None
OpBranchConditional %54 %57 %58
%57 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%61 = OpLoad %v4float %60
OpStore %55 %61
OpBranch %59
%58 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%64 = OpLoad %v4float %62
OpStore %55 %64
OpBranch %59
%59 = OpLabel
%65 = OpLoad %v4float %55
OpReturnValue %65
OpFunctionEnd
