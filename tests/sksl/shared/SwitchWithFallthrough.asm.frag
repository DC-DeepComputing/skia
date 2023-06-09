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
OpName %switch_fallthrough_twice_bi "switch_fallthrough_twice_bi"
OpName %ok "ok"
OpName %switch_fallthrough_with_vardecls_bi "switch_fallthrough_with_vardecls_bi"
OpName %a "a"
OpName %b "b"
OpName %main "main"
OpName %x "x"
OpName %_0_ok "_0_ok"
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
OpDecorate %41 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
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
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%27 = OpTypeFunction %bool %_ptr_Function_int
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%54 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%switch_fallthrough_twice_bi = OpFunction %bool None %27
%28 = OpFunctionParameter %_ptr_Function_int
%29 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %false
%33 = OpLoad %int %28
OpSelectionMerge %34 None
OpSwitch %33 %39 0 %35 1 %38 2 %38 3 %38
%35 = OpLabel
OpBranch %34
%38 = OpLabel
OpStore %ok %true
OpBranch %34
%39 = OpLabel
OpBranch %34
%34 = OpLabel
%41 = OpLoad %bool %ok
OpReturnValue %41
OpFunctionEnd
%switch_fallthrough_with_vardecls_bi = OpFunction %bool None %27
%42 = OpFunctionParameter %_ptr_Function_int
%43 = OpLabel
%a = OpVariable %_ptr_Function_bool Function
%b = OpVariable %_ptr_Function_bool Function
%46 = OpLoad %int %42
OpSelectionMerge %47 None
OpSwitch %46 %47 0 %49 1 %49 2 %50 3 %51
%49 = OpLabel
OpStore %b %true
OpBranch %50
%50 = OpLabel
%52 = OpLoad %bool %b
OpStore %a %52
OpBranch %51
%51 = OpLabel
%53 = OpLoad %bool %a
OpReturnValue %53
%47 = OpLabel
OpReturnValue %false
OpFunctionEnd
%main = OpFunction %v4float None %54
%55 = OpFunctionParameter %_ptr_Function_v2float
%56 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%_0_ok = OpVariable %_ptr_Function_bool Function
%73 = OpVariable %_ptr_Function_int Function
%78 = OpVariable %_ptr_Function_int Function
%81 = OpVariable %_ptr_Function_v4float Function
%58 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%61 = OpLoad %v4float %58
%62 = OpCompositeExtract %float %61 1
%63 = OpConvertFToS %int %62
OpStore %x %63
OpStore %_0_ok %false
OpSelectionMerge %65 None
OpSwitch %63 %69 2 %66 1 %68 0 %68
%66 = OpLabel
OpBranch %65
%68 = OpLabel
OpStore %_0_ok %true
OpBranch %65
%69 = OpLabel
OpBranch %65
%65 = OpLabel
%70 = OpLoad %bool %_0_ok
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
OpStore %73 %63
%74 = OpFunctionCall %bool %switch_fallthrough_twice_bi %73
OpBranch %72
%72 = OpLabel
%75 = OpPhi %bool %false %65 %74 %71
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
OpStore %78 %63
%79 = OpFunctionCall %bool %switch_fallthrough_with_vardecls_bi %78
OpBranch %77
%77 = OpLabel
%80 = OpPhi %bool %false %72 %79 %76
OpSelectionMerge %85 None
OpBranchConditional %80 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%87 = OpLoad %v4float %86
OpStore %81 %87
OpBranch %85
%84 = OpLabel
%88 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%90 = OpLoad %v4float %88
OpStore %81 %90
OpBranch %85
%85 = OpLabel
%91 = OpLoad %v4float %81
OpReturnValue %91
OpFunctionEnd
