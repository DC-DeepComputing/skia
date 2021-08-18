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
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_return_b "test_return_b"
OpName %test_break_b "test_break_b"
OpName %test_continue_b "test_continue_b"
OpName %test_if_return_b "test_if_return_b"
OpName %test_if_break_b "test_if_break_b"
OpName %test_else_b "test_else_b"
OpName %test_loop_return_b "test_loop_return_b"
OpName %x "x"
OpName %test_loop_break_b "test_loop_break_b"
OpName %x_0 "x"
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
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%120 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %25
%26 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%test_return_b = OpFunction %bool None %33
%34 = OpLabel
OpBranch %35
%35 = OpLabel
OpLoopMerge %39 %38 None
OpBranch %36
%36 = OpLabel
OpReturnValue %true
%37 = OpLabel
OpBranch %38
%38 = OpLabel
OpBranchConditional %false %35 %39
%39 = OpLabel
OpUnreachable
OpFunctionEnd
%test_break_b = OpFunction %bool None %33
%42 = OpLabel
OpBranch %43
%43 = OpLabel
OpLoopMerge %47 %46 None
OpBranch %44
%44 = OpLabel
OpBranch %47
%45 = OpLabel
OpBranch %46
%46 = OpLabel
OpBranchConditional %false %43 %47
%47 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_continue_b = OpFunction %bool None %33
%48 = OpLabel
OpBranch %49
%49 = OpLabel
OpLoopMerge %53 %52 None
OpBranch %50
%50 = OpLabel
OpBranch %52
%51 = OpLabel
OpBranch %52
%52 = OpLabel
OpBranchConditional %false %49 %53
%53 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_if_return_b = OpFunction %bool None %33
%54 = OpLabel
OpBranch %55
%55 = OpLabel
OpLoopMerge %59 %58 None
OpBranch %56
%56 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%64 = OpLoad %v4float %60
%65 = OpCompositeExtract %float %64 1
%66 = OpFOrdGreaterThan %bool %65 %float_0
OpSelectionMerge %69 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
OpReturnValue %true
%68 = OpLabel
OpBranch %59
%69 = OpLabel
OpBranch %58
%57 = OpLabel
OpBranch %58
%58 = OpLabel
OpBranchConditional %false %55 %59
%59 = OpLabel
OpReturnValue %false
OpFunctionEnd
%test_if_break_b = OpFunction %bool None %33
%70 = OpLabel
OpBranch %71
%71 = OpLabel
OpLoopMerge %75 %74 None
OpBranch %72
%72 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%77 = OpLoad %v4float %76
%78 = OpCompositeExtract %float %77 1
%79 = OpFOrdGreaterThan %bool %78 %float_0
OpSelectionMerge %82 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
OpBranch %75
%81 = OpLabel
OpBranch %74
%82 = OpLabel
OpBranch %73
%73 = OpLabel
OpBranch %74
%74 = OpLabel
OpBranchConditional %false %71 %75
%75 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_else_b = OpFunction %bool None %33
%83 = OpLabel
OpBranch %84
%84 = OpLabel
OpLoopMerge %88 %87 None
OpBranch %85
%85 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%90 = OpLoad %v4float %89
%91 = OpCompositeExtract %float %90 1
%92 = OpFOrdEqual %bool %91 %float_0
OpSelectionMerge %95 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
OpReturnValue %false
%94 = OpLabel
OpReturnValue %true
%95 = OpLabel
OpBranch %86
%86 = OpLabel
OpBranch %87
%87 = OpLabel
OpBranchConditional %false %84 %88
%88 = OpLabel
OpUnreachable
OpFunctionEnd
%test_loop_return_b = OpFunction %bool None %33
%96 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %99
%99 = OpLabel
OpLoopMerge %103 %102 None
OpBranch %100
%100 = OpLabel
%104 = OpLoad %int %x
%105 = OpSLessThan %bool %104 %int_0
OpBranchConditional %105 %101 %103
%101 = OpLabel
OpReturnValue %false
%102 = OpLabel
%107 = OpLoad %int %x
%108 = OpIAdd %int %107 %int_1
OpStore %x %108
OpBranch %99
%103 = OpLabel
OpReturnValue %true
OpFunctionEnd
%test_loop_break_b = OpFunction %bool None %33
%109 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %111
%111 = OpLabel
OpLoopMerge %115 %114 None
OpBranch %112
%112 = OpLabel
%116 = OpLoad %int %x_0
%117 = OpSLessThanEqual %bool %116 %int_1
OpBranchConditional %117 %113 %115
%113 = OpLabel
OpBranch %115
%114 = OpLabel
%118 = OpLoad %int %x_0
%119 = OpIAdd %int %118 %int_1
OpStore %x_0 %119
OpBranch %111
%115 = OpLabel
OpReturnValue %true
OpFunctionEnd
%main = OpFunction %v4float None %120
%121 = OpFunctionParameter %_ptr_Function_v2float
%122 = OpLabel
%152 = OpVariable %_ptr_Function_v4float Function
%123 = OpFunctionCall %bool %test_return_b
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpFunctionCall %bool %test_break_b
OpBranch %125
%125 = OpLabel
%127 = OpPhi %bool %false %122 %126 %124
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%130 = OpFunctionCall %bool %test_continue_b
OpBranch %129
%129 = OpLabel
%131 = OpPhi %bool %false %125 %130 %128
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpFunctionCall %bool %test_if_return_b
OpBranch %133
%133 = OpLabel
%135 = OpPhi %bool %false %129 %134 %132
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%138 = OpFunctionCall %bool %test_if_break_b
OpBranch %137
%137 = OpLabel
%139 = OpPhi %bool %false %133 %138 %136
OpSelectionMerge %141 None
OpBranchConditional %139 %140 %141
%140 = OpLabel
%142 = OpFunctionCall %bool %test_else_b
OpBranch %141
%141 = OpLabel
%143 = OpPhi %bool %false %137 %142 %140
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%146 = OpFunctionCall %bool %test_loop_return_b
OpBranch %145
%145 = OpLabel
%147 = OpPhi %bool %false %141 %146 %144
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%150 = OpFunctionCall %bool %test_loop_break_b
OpBranch %149
%149 = OpLabel
%151 = OpPhi %bool %false %145 %150 %148
OpSelectionMerge %156 None
OpBranchConditional %151 %154 %155
%154 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %20 %int_0
%158 = OpLoad %v4float %157
OpStore %152 %158
OpBranch %156
%155 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %20 %int_1
%160 = OpLoad %v4float %159
OpStore %152 %160
OpBranch %156
%156 = OpLabel
%161 = OpLoad %v4float %152
OpReturnValue %161
OpFunctionEnd
