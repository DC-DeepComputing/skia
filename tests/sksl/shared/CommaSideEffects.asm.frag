OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorRed"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorWhite"
OpMemberName %_UniformBuffer 3 "colorBlack"
OpName %_entrypoint_v "_entrypoint_v"
OpName %setToColorBlack_vh4 "setToColorBlack_vh4"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %d RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%25 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%33 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%setToColorBlack_vh4 = OpFunction %void None %25
%26 = OpFunctionParameter %_ptr_Function_v4float
%27 = OpLabel
%28 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%32 = OpLoad %v4float %28
OpStore %26 %32
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %33
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%d = OpVariable %_ptr_Function_v4float Function
%46 = OpVariable %_ptr_Function_v4float Function
%86 = OpVariable %_ptr_Function_v4float Function
%40 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%42 = OpLoad %v4float %40
OpStore %b %42
%43 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%45 = OpLoad %v4float %43
OpStore %c %45
%47 = OpFunctionCall %void %setToColorBlack_vh4 %46
%48 = OpLoad %v4float %46
OpStore %d %48
%49 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%51 = OpLoad %v4float %49
OpStore %a %51
%52 = OpFMul %v4float %51 %51
OpStore %a %52
%53 = OpFMul %v4float %42 %42
OpStore %b %53
%54 = OpFMul %v4float %45 %45
OpStore %c %54
%55 = OpFMul %v4float %48 %48
OpStore %d %55
%57 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%58 = OpLoad %v4float %57
%59 = OpFOrdEqual %v4bool %52 %58
%61 = OpAll %bool %59
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%64 = OpLoad %v4float %b
%65 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%66 = OpLoad %v4float %65
%67 = OpFOrdEqual %v4bool %64 %66
%68 = OpAll %bool %67
OpBranch %63
%63 = OpLabel
%69 = OpPhi %bool %false %35 %68 %62
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpLoad %v4float %c
%73 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%74 = OpLoad %v4float %73
%75 = OpFOrdEqual %v4bool %72 %74
%76 = OpAll %bool %75
OpBranch %71
%71 = OpLabel
%77 = OpPhi %bool %false %63 %76 %70
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %v4float %d
%81 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%82 = OpLoad %v4float %81
%83 = OpFOrdEqual %v4bool %80 %82
%84 = OpAll %bool %83
OpBranch %79
%79 = OpLabel
%85 = OpPhi %bool %false %71 %84 %78
OpSelectionMerge %89 None
OpBranchConditional %85 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%91 = OpLoad %v4float %90
OpStore %86 %91
OpBranch %89
%88 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%93 = OpLoad %v4float %92
OpStore %86 %93
OpBranch %89
%89 = OpLabel
%94 = OpLoad %v4float %86
OpReturnValue %94
OpFunctionEnd
