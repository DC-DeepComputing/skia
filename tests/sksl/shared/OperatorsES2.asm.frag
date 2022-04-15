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
OpDecorate %70 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
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
%134 = OpVariable %_ptr_Function_v4float Function
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
%54 = OpIMul %int %53 %int_3
%56 = OpIAdd %int %54 %int_4
%57 = OpISub %int %56 %int_2
OpStore %z %57
%62 = OpFOrdGreaterThan %bool %47 %float_4
%63 = OpFOrdLessThan %bool %47 %float_2
%64 = OpLogicalEqual %bool %62 %63
OpSelectionMerge %66 None
OpBranchConditional %64 %66 %65
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_float %10 %int_2
%70 = OpLoad %float %68
%71 = OpFOrdGreaterThanEqual %bool %float_2 %70
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%74 = OpFOrdLessThanEqual %bool %50 %47
OpBranch %73
%73 = OpLabel
%75 = OpPhi %bool %false %65 %74 %72
OpBranch %66
%66 = OpLabel
%76 = OpPhi %bool %true %25 %75 %73
OpStore %b %76
%78 = OpAccessChain %_ptr_Uniform_float %10 %int_2
%79 = OpLoad %float %78
%80 = OpFOrdGreaterThan %bool %79 %float_2
OpStore %c %80
%82 = OpLoad %bool %b
%83 = OpLoad %bool %c
%84 = OpLogicalNotEqual %bool %82 %83
OpStore %d %84
%86 = OpLoad %bool %b
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %bool %c
OpBranch %88
%88 = OpLabel
%90 = OpPhi %bool %false %66 %89 %87
OpStore %e %90
%92 = OpLoad %bool %b
OpSelectionMerge %94 None
OpBranchConditional %92 %94 %93
%93 = OpLabel
%95 = OpLoad %bool %c
OpBranch %94
%94 = OpLabel
%96 = OpPhi %bool %true %88 %95 %93
OpStore %f %96
%97 = OpLoad %float %x
%99 = OpFAdd %float %97 %float_12
OpStore %x %99
%100 = OpFSub %float %99 %float_12
OpStore %x %100
%101 = OpLoad %float %y
%103 = OpFDiv %float %101 %float_10
OpStore %y %103
%104 = OpFMul %float %100 %103
OpStore %x %104
OpStore %x %float_6
%106 = OpLoad %bool %b
%107 = OpSelect %float %106 %float_1 %float_0
%108 = OpLoad %bool %c
%109 = OpSelect %float %108 %float_1 %float_0
%110 = OpFMul %float %107 %109
%111 = OpLoad %bool %d
%112 = OpSelect %float %111 %float_1 %float_0
%113 = OpFMul %float %110 %112
%114 = OpLoad %bool %e
%115 = OpSelect %float %114 %float_1 %float_0
%116 = OpFMul %float %113 %115
%117 = OpLoad %bool %f
%118 = OpSelect %float %117 %float_1 %float_0
%119 = OpFMul %float %116 %118
OpStore %y %119
OpStore %y %float_6
%120 = OpLoad %int %z
%122 = OpISub %int %120 %int_1
OpStore %z %122
OpStore %z %int_6
%124 = OpFOrdEqual %bool %float_6 %float_6
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%127 = OpFOrdEqual %bool %float_6 %float_6
OpBranch %126
%126 = OpLabel
%128 = OpPhi %bool %false %94 %127 %125
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpLoad %int %z
%132 = OpIEqual %bool %131 %int_6
OpBranch %130
%130 = OpLabel
%133 = OpPhi %bool %false %126 %132 %129
OpSelectionMerge %138 None
OpBranchConditional %133 %136 %137
%136 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%142 = OpLoad %v4float %139
OpStore %134 %142
OpBranch %138
%137 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%144 = OpLoad %v4float %143
OpStore %134 %144
OpBranch %138
%138 = OpLabel
%145 = OpLoad %v4float %134
OpReturnValue %145
OpFunctionEnd
