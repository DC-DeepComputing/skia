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
OpName %return_on_both_sides_b "return_on_both_sides_b"
OpName %for_inside_body_b "for_inside_body_b"
OpName %x "x"
OpName %after_for_body_b "after_for_body_b"
OpName %x_0 "x"
OpName %for_with_double_sided_conditional_return_b "for_with_double_sided_conditional_return_b"
OpName %x_1 "x"
OpName %if_else_chain_b "if_else_chain_b"
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
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %17 Binding 0
OpDecorate %17 DescriptorSet 0
OpDecorate %36 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
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
%17 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%22 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%26 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%30 = OpTypeFunction %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%true = OpConstantTrue %bool
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%115 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %22
%23 = OpLabel
%27 = OpVariable %_ptr_Function_v2float Function
OpStore %27 %26
%29 = OpFunctionCall %v4float %main %27
OpStore %sk_FragColor %29
OpReturn
OpFunctionEnd
%return_on_both_sides_b = OpFunction %bool None %30
%31 = OpLabel
%32 = OpAccessChain %_ptr_Uniform_float %17 %int_2
%36 = OpLoad %float %32
%38 = OpFOrdEqual %bool %36 %float_1
OpSelectionMerge %41 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
OpReturnValue %true
%40 = OpLabel
OpReturnValue %true
%41 = OpLabel
OpUnreachable
OpFunctionEnd
%for_inside_body_b = OpFunction %bool None %30
%43 = OpLabel
%x = OpVariable %_ptr_Function_int Function
OpStore %x %int_0
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
%52 = OpLoad %int %x
%54 = OpSLessThanEqual %bool %52 %int_10
OpBranchConditional %54 %49 %51
%49 = OpLabel
OpReturnValue %true
%50 = OpLabel
%56 = OpLoad %int %x
%57 = OpIAdd %int %56 %int_1
OpStore %x %57
OpBranch %47
%51 = OpLabel
OpUnreachable
OpFunctionEnd
%after_for_body_b = OpFunction %bool None %30
%58 = OpLabel
%x_0 = OpVariable %_ptr_Function_int Function
OpStore %x_0 %int_0
OpBranch %60
%60 = OpLabel
OpLoopMerge %64 %63 None
OpBranch %61
%61 = OpLabel
%65 = OpLoad %int %x_0
%66 = OpSLessThanEqual %bool %65 %int_10
OpBranchConditional %66 %62 %64
%62 = OpLabel
OpBranch %63
%63 = OpLabel
%67 = OpLoad %int %x_0
%68 = OpIAdd %int %67 %int_1
OpStore %x_0 %68
OpBranch %60
%64 = OpLabel
OpReturnValue %true
OpFunctionEnd
%for_with_double_sided_conditional_return_b = OpFunction %bool None %30
%69 = OpLabel
%x_1 = OpVariable %_ptr_Function_int Function
OpStore %x_1 %int_0
OpBranch %71
%71 = OpLabel
OpLoopMerge %75 %74 None
OpBranch %72
%72 = OpLabel
%76 = OpLoad %int %x_1
%77 = OpSLessThanEqual %bool %76 %int_10
OpBranchConditional %77 %73 %75
%73 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_float %17 %int_2
%79 = OpLoad %float %78
%80 = OpFOrdEqual %bool %79 %float_1
OpSelectionMerge %83 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
OpReturnValue %true
%82 = OpLabel
OpReturnValue %true
%83 = OpLabel
OpBranch %74
%74 = OpLabel
%84 = OpLoad %int %x_1
%85 = OpIAdd %int %84 %int_1
OpStore %x_1 %85
OpBranch %71
%75 = OpLabel
OpUnreachable
OpFunctionEnd
%if_else_chain_b = OpFunction %bool None %30
%86 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_float %17 %int_2
%88 = OpLoad %float %87
%89 = OpFOrdEqual %bool %88 %float_1
OpSelectionMerge %92 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
OpReturnValue %true
%91 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_float %17 %int_2
%94 = OpLoad %float %93
%96 = OpFOrdEqual %bool %94 %float_2
OpSelectionMerge %99 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
OpReturnValue %false
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_float %17 %int_2
%102 = OpLoad %float %101
%104 = OpFOrdEqual %bool %102 %float_3
OpSelectionMerge %107 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
OpReturnValue %true
%106 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_float %17 %int_2
%109 = OpLoad %float %108
%111 = OpFOrdEqual %bool %109 %float_4
OpSelectionMerge %114 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
OpReturnValue %false
%113 = OpLabel
OpReturnValue %true
%114 = OpLabel
OpBranch %107
%107 = OpLabel
OpBranch %99
%99 = OpLabel
OpBranch %92
%92 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %115
%116 = OpFunctionParameter %_ptr_Function_v2float
%117 = OpLabel
%138 = OpVariable %_ptr_Function_v4float Function
OpSelectionMerge %119 None
OpBranchConditional %true %118 %119
%118 = OpLabel
%120 = OpFunctionCall %bool %return_on_both_sides_b
OpBranch %119
%119 = OpLabel
%121 = OpPhi %bool %false %117 %120 %118
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpFunctionCall %bool %for_inside_body_b
OpBranch %123
%123 = OpLabel
%125 = OpPhi %bool %false %119 %124 %122
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%128 = OpFunctionCall %bool %after_for_body_b
OpBranch %127
%127 = OpLabel
%129 = OpPhi %bool %false %123 %128 %126
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%132 = OpFunctionCall %bool %for_with_double_sided_conditional_return_b
OpBranch %131
%131 = OpLabel
%133 = OpPhi %bool %false %127 %132 %130
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%136 = OpFunctionCall %bool %if_else_chain_b
OpBranch %135
%135 = OpLabel
%137 = OpPhi %bool %false %131 %136 %134
OpSelectionMerge %142 None
OpBranchConditional %137 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %17 %int_0
%145 = OpLoad %v4float %143
OpStore %138 %145
OpBranch %142
%141 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %17 %int_1
%147 = OpLoad %v4float %146
OpStore %138 %147
OpBranch %142
%142 = OpLabel
%148 = OpLoad %v4float %138
OpReturnValue %148
OpFunctionEnd
