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
OpDecorate %65 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
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
%85 = OpVariable %_ptr_Function_v4float Function
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
%64 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%65 = OpLoad %v4float %64
%66 = OpFOrdEqual %v4bool %53 %65
%67 = OpAll %bool %66
OpBranch %63
%63 = OpLabel
%68 = OpPhi %bool %false %35 %67 %62
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %v4float %c
%72 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%73 = OpLoad %v4float %72
%74 = OpFOrdEqual %v4bool %71 %73
%75 = OpAll %bool %74
OpBranch %70
%70 = OpLabel
%76 = OpPhi %bool %false %63 %75 %69
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpLoad %v4float %d
%80 = OpAccessChain %_ptr_Uniform_v4float %11 %int_3
%81 = OpLoad %v4float %80
%82 = OpFOrdEqual %v4bool %79 %81
%83 = OpAll %bool %82
OpBranch %78
%78 = OpLabel
%84 = OpPhi %bool %false %70 %83 %77
OpSelectionMerge %88 None
OpBranchConditional %84 %86 %87
%86 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%90 = OpLoad %v4float %89
OpStore %85 %90
OpBranch %88
%87 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%92 = OpLoad %v4float %91
OpStore %85 %92
OpBranch %88
%88 = OpLabel
%93 = OpLoad %v4float %85
OpReturnValue %93
OpFunctionEnd
