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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_1 = OpConstant %int 1
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_5 = OpConstant %int 5
%float_6 = OpConstant %float 6
%int_6 = OpConstant %int 6
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
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%104 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%37 = OpLoad %float %x
%38 = OpLoad %float %x
%39 = OpFSub %float %37 %38
%40 = OpLoad %float %y
%41 = OpLoad %float %x
%42 = OpFMul %float %40 %41
%43 = OpLoad %float %x
%44 = OpFMul %float %42 %43
%45 = OpLoad %float %y
%46 = OpLoad %float %x
%47 = OpFSub %float %45 %46
%48 = OpFMul %float %44 %47
%49 = OpFAdd %float %39 %48
OpStore %x %49
%50 = OpLoad %float %x
%51 = OpLoad %float %y
%52 = OpFDiv %float %50 %51
%53 = OpLoad %float %x
%54 = OpFDiv %float %52 %53
OpStore %y %54
%55 = OpLoad %int %z
%57 = OpSDiv %int %55 %int_2
%58 = OpSMod %int %57 %int_3
%60 = OpShiftLeftLogical %int %58 %int_4
%61 = OpShiftRightArithmetic %int %60 %int_2
%63 = OpShiftLeftLogical %int %61 %int_1
OpStore %z %63
%64 = OpLoad %float %x
%66 = OpFAdd %float %64 %float_12
OpStore %x %66
%67 = OpLoad %float %x
%68 = OpFSub %float %67 %float_12
OpStore %x %68
%69 = OpLoad %float %x
%70 = OpLoad %float %y
%72 = OpFDiv %float %70 %float_10
OpStore %y %72
%73 = OpFMul %float %69 %72
OpStore %x %73
%74 = OpLoad %int %z
%76 = OpBitwiseOr %int %74 %int_0
OpStore %z %76
%77 = OpLoad %int %z
%79 = OpBitwiseAnd %int %77 %int_n1
OpStore %z %79
%80 = OpLoad %int %z
%81 = OpBitwiseXor %int %80 %int_0
OpStore %z %81
%82 = OpLoad %int %z
%83 = OpShiftRightArithmetic %int %82 %int_2
OpStore %z %83
%84 = OpLoad %int %z
%85 = OpShiftLeftLogical %int %84 %int_4
OpStore %z %85
%86 = OpLoad %int %z
%88 = OpSMod %int %86 %int_5
OpStore %z %88
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%92 = OpLoad %float %x
%93 = OpFOrdEqual %bool %92 %float_6
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpLoad %float %y
%97 = OpFOrdEqual %bool %96 %float_6
OpBranch %95
%95 = OpLabel
%98 = OpPhi %bool %false %27 %97 %94
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %int %z
%102 = OpIEqual %bool %101 %int_6
OpBranch %100
%100 = OpLabel
%103 = OpPhi %bool %false %95 %102 %99
OpSelectionMerge %108 None
OpBranchConditional %103 %106 %107
%106 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%111 = OpLoad %v4float %109
OpStore %104 %111
OpBranch %108
%107 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%113 = OpLoad %v4float %112
OpStore %104 %113
OpBranch %108
%108 = OpLabel
%114 = OpLoad %v4float %104
OpReturnValue %114
OpFunctionEnd
