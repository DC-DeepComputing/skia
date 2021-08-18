OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %scratchVar "scratchVar"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_flat_b "test_flat_b"
OpName %test_if_b "test_if_b"
OpName %test_else_b "test_else_b"
OpName %test_loop_if_b "test_loop_if_b"
OpName %x "x"
OpName %main "main"
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
OpDecorate %20 Binding 0
OpDecorate %20 DescriptorSet 0
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%scratchVar = OpVariable %_ptr_Private_int Private
%int_0 = OpConstant %int 0
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%20 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%25 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%false = OpConstantFalse %bool
%_ptr_Function_int = OpTypePointer Function %int
%80 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %25
%26 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%test_flat_b = OpFunction %bool None %33
%34 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_b = OpFunction %bool None %33
%36 = OpLabel
%37 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%39 = OpLoad %v4float %37
%40 = OpCompositeExtract %float %39 1
%41 = OpFOrdGreaterThan %bool %40 %float_0
OpSelectionMerge %44 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
OpReturnValue %true
%43 = OpLabel
%46 = OpLoad %int %scratchVar
%47 = OpIAdd %int %46 %int_1
OpStore %scratchVar %47
OpBranch %44
%44 = OpLabel
%48 = OpLoad %int %scratchVar
%49 = OpIAdd %int %48 %int_1
OpStore %scratchVar %49
OpReturnValue %false
OpFunctionEnd
%test_else_b = OpFunction %bool None %33
%51 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%53 = OpLoad %v4float %52
%54 = OpCompositeExtract %float %53 1
%55 = OpFOrdEqual %bool %54 %float_0
OpSelectionMerge %58 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
OpReturnValue %false
%57 = OpLabel
OpReturnValue %true
%58 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_if_b = OpFunction %bool None %33
%59 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %62
%62 = OpLabel
OpLoopMerge %66 %65 None
OpBranch %63
%63 = OpLabel
%67 = OpLoad %int %x
%68 = OpSLessThanEqual %bool %67 %int_1
OpBranchConditional %68 %64 %66
%64 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%70 = OpLoad %v4float %69
%71 = OpCompositeExtract %float %70 1
%72 = OpFOrdEqual %bool %71 %float_0
OpSelectionMerge %75 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
OpReturnValue %false
%74 = OpLabel
OpReturnValue %true
%75 = OpLabel
OpBranch %65
%65 = OpLabel
%76 = OpLoad %int %x
%77 = OpIAdd %int %76 %int_1
OpStore %x %77
OpBranch %62
%66 = OpLabel
%78 = OpLoad %int %scratchVar
%79 = OpIAdd %int %78 %int_1
OpStore %scratchVar %79
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %80
%81 = OpFunctionParameter %_ptr_Function_v2float
%82 = OpLabel
%96 = OpVariable %_ptr_Function_v4float Function
OpStore %scratchVar %int_0
%83 = OpFunctionCall %bool %test_flat_b
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpFunctionCall %bool %test_if_b
OpBranch %85
%85 = OpLabel
%87 = OpPhi %bool %false %82 %86 %84
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpFunctionCall %bool %test_else_b
OpBranch %89
%89 = OpLabel
%91 = OpPhi %bool %false %85 %90 %88
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpFunctionCall %bool %test_loop_if_b
OpBranch %93
%93 = OpLabel
%95 = OpPhi %bool %false %89 %94 %92
OpSelectionMerge %100 None
OpBranchConditional %95 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%102 = OpLoad %v4float %101
OpStore %96 %102
OpBranch %100
%99 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %20 %int_1
%104 = OpLoad %v4float %103
OpStore %96 %104
OpBranch %100
%100 = OpLabel
%105 = OpLoad %v4float %96
OpReturnValue %105
OpFunctionEnd
