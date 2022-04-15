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
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %61 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%float_6 = OpConstant %float 6
%int_1 = OpConstant %int 1
%int_6 = OpConstant %int 6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%b = OpVariable %_ptr_Function_bool Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
%f = OpVariable %_ptr_Function_bool Function
%124 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
%35 = OpFSub %float %float_1 %float_1
%36 = OpFMul %float %float_2 %float_1
%37 = OpFMul %float %36 %float_1
%38 = OpFSub %float %float_2 %float_1
%39 = OpFMul %float %37 %38
%40 = OpFAdd %float %35 %39
OpStore %x %40
%41 = OpFDiv %float %40 %float_2
%42 = OpFDiv %float %41 %40
OpStore %y %42
%44 = OpSDiv %int %int_3 %int_2
%45 = OpIMul %int %44 %int_3
%47 = OpIAdd %int %45 %int_4
%48 = OpISub %int %47 %int_2
OpStore %z %48
%53 = OpFOrdGreaterThan %bool %40 %float_4
%54 = OpFOrdLessThan %bool %40 %float_2
%55 = OpLogicalEqual %bool %53 %54
OpSelectionMerge %57 None
OpBranchConditional %55 %57 %56
%56 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_float %10 %int_2
%61 = OpLoad %float %59
%62 = OpFOrdGreaterThanEqual %bool %float_2 %61
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%65 = OpLoad %float %y
%66 = OpLoad %float %x
%67 = OpFOrdLessThanEqual %bool %65 %66
OpBranch %64
%64 = OpLabel
%68 = OpPhi %bool %false %56 %67 %63
OpBranch %57
%57 = OpLabel
%69 = OpPhi %bool %true %25 %68 %64
OpStore %b %69
%71 = OpAccessChain %_ptr_Uniform_float %10 %int_2
%72 = OpLoad %float %71
%73 = OpFOrdGreaterThan %bool %72 %float_2
OpStore %c %73
%75 = OpLogicalNotEqual %bool %69 %73
OpStore %d %75
OpSelectionMerge %78 None
OpBranchConditional %69 %77 %78
%77 = OpLabel
%79 = OpLoad %bool %c
OpBranch %78
%78 = OpLabel
%80 = OpPhi %bool %false %57 %79 %77
OpStore %e %80
%82 = OpLoad %bool %b
OpSelectionMerge %84 None
OpBranchConditional %82 %84 %83
%83 = OpLabel
%85 = OpLoad %bool %c
OpBranch %84
%84 = OpLabel
%86 = OpPhi %bool %true %78 %85 %83
OpStore %f %86
%87 = OpLoad %float %x
%89 = OpFAdd %float %87 %float_12
OpStore %x %89
%90 = OpFSub %float %89 %float_12
OpStore %x %90
%91 = OpLoad %float %y
%93 = OpFDiv %float %91 %float_10
OpStore %y %93
%94 = OpFMul %float %90 %93
OpStore %x %94
OpStore %x %float_6
%96 = OpLoad %bool %b
%97 = OpSelect %float %96 %float_1 %float_0
%98 = OpLoad %bool %c
%99 = OpSelect %float %98 %float_1 %float_0
%100 = OpFMul %float %97 %99
%101 = OpLoad %bool %d
%102 = OpSelect %float %101 %float_1 %float_0
%103 = OpFMul %float %100 %102
%104 = OpLoad %bool %e
%105 = OpSelect %float %104 %float_1 %float_0
%106 = OpFMul %float %103 %105
%107 = OpSelect %float %86 %float_1 %float_0
%108 = OpFMul %float %106 %107
OpStore %y %108
OpStore %y %float_6
%109 = OpLoad %int %z
%111 = OpISub %int %109 %int_1
OpStore %z %111
OpStore %z %int_6
%113 = OpFOrdEqual %bool %float_6 %float_6
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %float %y
%117 = OpFOrdEqual %bool %116 %float_6
OpBranch %115
%115 = OpLabel
%118 = OpPhi %bool %false %84 %117 %114
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpLoad %int %z
%122 = OpIEqual %bool %121 %int_6
OpBranch %120
%120 = OpLabel
%123 = OpPhi %bool %false %115 %122 %119
OpSelectionMerge %128 None
OpBranchConditional %123 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%132 = OpLoad %v4float %129
OpStore %124 %132
OpBranch %128
%127 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%134 = OpLoad %v4float %133
OpStore %124 %134
OpBranch %128
%128 = OpLabel
%135 = OpLoad %v4float %124
OpReturnValue %135
OpFunctionEnd
