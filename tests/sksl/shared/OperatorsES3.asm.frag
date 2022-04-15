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
OpMemberName %_UniformBuffer 2 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%w = OpVariable %_ptr_Function_v2int Function
%107 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%35 = OpLoad %float %x
%36 = OpLoad %float %x
%37 = OpFSub %float %35 %36
%38 = OpLoad %float %y
%39 = OpLoad %float %x
%40 = OpFMul %float %38 %39
%41 = OpLoad %float %x
%42 = OpFMul %float %40 %41
%43 = OpLoad %float %y
%44 = OpLoad %float %x
%45 = OpFSub %float %43 %44
%46 = OpFMul %float %42 %45
%47 = OpFAdd %float %37 %46
OpStore %x %47
%48 = OpLoad %float %y
%49 = OpFDiv %float %47 %48
%50 = OpFDiv %float %49 %47
OpStore %y %50
%51 = OpLoad %int %z
%53 = OpSDiv %int %51 %int_2
%54 = OpSMod %int %53 %int_3
%56 = OpShiftLeftLogical %int %54 %int_4
%57 = OpShiftRightArithmetic %int %56 %int_2
%59 = OpShiftLeftLogical %int %57 %int_1
OpStore %z %59
%61 = OpFAdd %float %47 %float_12
OpStore %x %61
%62 = OpFSub %float %61 %float_12
OpStore %x %62
%64 = OpFDiv %float %50 %float_10
OpStore %y %64
%65 = OpFMul %float %62 %64
OpStore %x %65
%67 = OpBitwiseOr %int %59 %int_0
OpStore %z %67
%69 = OpBitwiseAnd %int %67 %int_n1
OpStore %z %69
%70 = OpBitwiseXor %int %69 %int_0
OpStore %z %70
%71 = OpShiftRightArithmetic %int %70 %int_2
OpStore %z %71
%72 = OpShiftLeftLogical %int %71 %int_4
OpStore %z %72
%74 = OpSMod %int %72 %int_5
OpStore %z %74
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%80 = OpNot %int %int_5
%81 = OpCompositeConstruct %v2int %80 %80
OpStore %w %81
%83 = OpLoad %v2int %w
%82 = OpNot %v2int %83
OpStore %w %82
%85 = OpCompositeExtract %int %82 0
%86 = OpIEqual %bool %85 %int_5
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpCompositeExtract %int %82 1
%90 = OpIEqual %bool %89 %int_5
OpBranch %88
%88 = OpLabel
%91 = OpPhi %bool %false %25 %90 %87
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %float %x
%95 = OpFOrdEqual %bool %94 %float_6
OpBranch %93
%93 = OpLabel
%96 = OpPhi %bool %false %88 %95 %92
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %float %y
%100 = OpFOrdEqual %bool %99 %float_6
OpBranch %98
%98 = OpLabel
%101 = OpPhi %bool %false %93 %100 %97
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpLoad %int %z
%105 = OpIEqual %bool %104 %int_6
OpBranch %103
%103 = OpLabel
%106 = OpPhi %bool %false %98 %105 %102
OpSelectionMerge %111 None
OpBranchConditional %106 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%114 = OpLoad %v4float %112
OpStore %107 %114
OpBranch %111
%110 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%116 = OpLoad %v4float %115
OpStore %107 %116
OpBranch %111
%111 = OpLabel
%117 = OpLoad %v4float %107
OpReturnValue %117
OpFunctionEnd
