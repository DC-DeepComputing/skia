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
OpMemberName %_UniformBuffer 2 "mask"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpName %mask "mask"
OpName %imask "imask"
OpName %m "m"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %m RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_UniformBuffer = OpTypeStruct %v4float %v4float %int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%uint = OpTypeInt 32 0
%v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_2 = OpConstant %int 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%mask = OpVariable %_ptr_Function_v2uint Function
%imask = OpVariable %_ptr_Function_v2int Function
%m = OpVariable %_ptr_Function_mat4v4float Function
%88 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
OpSelectionMerge %32 None
OpBranchConditional %true %31 %32
%31 = OpLabel
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %34
%38 = OpCompositeExtract %float %37 1
%40 = OpFOrdEqual %bool %38 %float_1
%33 = OpLogicalNot %bool %40
OpBranch %32
%32 = OpLabel
%41 = OpPhi %bool %false %26 %33 %31
OpStore %ok %41
%46 = OpAccessChain %_ptr_Uniform_int %10 %int_2
%49 = OpLoad %int %46
%50 = OpBitcast %uint %49
%52 = OpAccessChain %_ptr_Uniform_int %10 %int_2
%53 = OpLoad %int %52
%54 = OpBitcast %uint %53
%51 = OpNot %uint %54
%55 = OpCompositeConstruct %v2uint %50 %51
OpStore %mask %55
%59 = OpNot %v2uint %55
%60 = OpCompositeExtract %uint %59 0
%61 = OpBitcast %int %60
%62 = OpCompositeExtract %uint %59 1
%63 = OpBitcast %int %62
%64 = OpCompositeConstruct %v2int %61 %63
OpStore %imask %64
%65 = OpNot %v2uint %55
%66 = OpNot %v2int %64
%67 = OpCompositeExtract %int %66 0
%68 = OpBitcast %uint %67
%69 = OpCompositeExtract %int %66 1
%70 = OpBitcast %uint %69
%71 = OpCompositeConstruct %v2uint %68 %70
%72 = OpBitwiseAnd %v2uint %65 %71
OpStore %mask %72
%76 = OpCompositeExtract %uint %72 0
%77 = OpConvertUToF %float %76
%78 = OpCompositeConstruct %v4float %77 %float_0 %float_0 %float_0
%79 = OpCompositeConstruct %v4float %float_0 %77 %float_0 %float_0
%80 = OpCompositeConstruct %v4float %float_0 %float_0 %77 %float_0
%81 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %77
%82 = OpCompositeConstruct %mat4v4float %78 %79 %80 %81
OpStore %m %82
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdEqual %bool %86 %float_1
%83 = OpLogicalNot %bool %87
OpSelectionMerge %92 None
OpBranchConditional %83 %90 %91
%90 = OpLabel
%93 = OpFNegate %v4float %78
%94 = OpFNegate %v4float %79
%95 = OpFNegate %v4float %80
%96 = OpFNegate %v4float %81
%97 = OpCompositeConstruct %mat4v4float %93 %94 %95 %96
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpFNegate %v4float %99
%101 = OpMatrixTimesVector %v4float %97 %100
OpStore %88 %101
OpBranch %92
%91 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%104 = OpLoad %v4float %102
OpStore %88 %104
OpBranch %92
%92 = OpLabel
%105 = OpLoad %v4float %88
OpReturnValue %105
OpFunctionEnd
