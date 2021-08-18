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
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
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
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %76 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
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
%b = OpVariable %_ptr_Function_bool Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
%f = OpVariable %_ptr_Function_bool Function
%146 = OpVariable %_ptr_Function_v4float Function
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
%58 = OpIMul %int %57 %int_3
%60 = OpIAdd %int %58 %int_4
%61 = OpISub %int %60 %int_2
OpStore %z %61
%65 = OpLoad %float %x
%67 = OpFOrdGreaterThan %bool %65 %float_4
%68 = OpLoad %float %x
%69 = OpFOrdLessThan %bool %68 %float_2
%70 = OpLogicalEqual %bool %67 %69
OpSelectionMerge %72 None
OpBranchConditional %70 %72 %71
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_float %12 %int_2
%76 = OpLoad %float %74
%77 = OpFOrdGreaterThanEqual %bool %float_2 %76
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpLoad %float %y
%81 = OpLoad %float %x
%82 = OpFOrdLessThanEqual %bool %80 %81
OpBranch %79
%79 = OpLabel
%83 = OpPhi %bool %false %71 %82 %78
OpBranch %72
%72 = OpLabel
%84 = OpPhi %bool %true %27 %83 %79
OpStore %b %84
%86 = OpAccessChain %_ptr_Uniform_float %12 %int_2
%87 = OpLoad %float %86
%88 = OpFOrdGreaterThan %bool %87 %float_2
OpStore %c %88
%90 = OpLoad %bool %b
%91 = OpLoad %bool %c
%92 = OpLogicalNotEqual %bool %90 %91
OpStore %d %92
%94 = OpLoad %bool %b
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %bool %c
OpBranch %96
%96 = OpLabel
%98 = OpPhi %bool %false %72 %97 %95
OpStore %e %98
%100 = OpLoad %bool %b
OpSelectionMerge %102 None
OpBranchConditional %100 %102 %101
%101 = OpLabel
%103 = OpLoad %bool %c
OpBranch %102
%102 = OpLabel
%104 = OpPhi %bool %true %96 %103 %101
OpStore %f %104
%105 = OpLoad %float %x
%107 = OpFAdd %float %105 %float_12
OpStore %x %107
%108 = OpLoad %float %x
%109 = OpFSub %float %108 %float_12
OpStore %x %109
%110 = OpLoad %float %x
%111 = OpLoad %float %y
%113 = OpFDiv %float %111 %float_10
OpStore %y %113
%114 = OpFMul %float %110 %113
OpStore %x %114
OpStore %x %float_6
%116 = OpLoad %bool %b
%117 = OpSelect %float %116 %float_1 %float_0
%118 = OpLoad %bool %c
%119 = OpSelect %float %118 %float_1 %float_0
%120 = OpFMul %float %117 %119
%121 = OpLoad %bool %d
%122 = OpSelect %float %121 %float_1 %float_0
%123 = OpFMul %float %120 %122
%124 = OpLoad %bool %e
%125 = OpSelect %float %124 %float_1 %float_0
%126 = OpFMul %float %123 %125
%127 = OpLoad %bool %f
%128 = OpSelect %float %127 %float_1 %float_0
%129 = OpFMul %float %126 %128
OpStore %y %129
OpStore %y %float_6
%130 = OpLoad %int %z
%132 = OpISub %int %130 %int_1
OpStore %z %132
OpStore %z %int_6
%134 = OpLoad %float %x
%135 = OpFOrdEqual %bool %134 %float_6
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpLoad %float %y
%139 = OpFOrdEqual %bool %138 %float_6
OpBranch %137
%137 = OpLabel
%140 = OpPhi %bool %false %102 %139 %136
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpLoad %int %z
%144 = OpIEqual %bool %143 %int_6
OpBranch %142
%142 = OpLabel
%145 = OpPhi %bool %false %137 %144 %141
OpSelectionMerge %150 None
OpBranchConditional %145 %148 %149
%148 = OpLabel
%151 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%154 = OpLoad %v4float %151
OpStore %146 %154
OpBranch %150
%149 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%156 = OpLoad %v4float %155
OpStore %146 %156
OpBranch %150
%150 = OpLabel
%157 = OpLoad %v4float %146
OpReturnValue %157
OpFunctionEnd
