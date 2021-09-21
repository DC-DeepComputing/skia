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
OpName %_entrypoint_v "_entrypoint_v"
OpName %switch_with_continue_in_loop_bi "switch_with_continue_in_loop_bi"
OpName %val "val"
OpName %i "i"
OpName %loop_with_break_in_switch_bi "loop_with_break_in_switch_bi"
OpName %val_0 "val"
OpName %i_0 "i"
OpName %loop_with_continue_in_switch_bi "loop_with_continue_in_switch_bi"
OpName %val_1 "val"
OpName %i_1 "i"
OpName %main "main"
OpName %x "x"
OpName %_0_val "_0_val"
OpName %_1_i "_1_i"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%27 = OpTypeFunction %bool %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%int_11 = OpConstant %int 11
%false = OpConstantFalse %bool
%int_20 = OpConstant %int 20
%107 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%switch_with_continue_in_loop_bi = OpFunction %bool None %27
%29 = OpFunctionParameter %_ptr_Function_int
%30 = OpLabel
%val = OpVariable %_ptr_Function_int Function
%i = OpVariable %_ptr_Function_int Function
OpStore %val %int_0
%33 = OpLoad %int %29
OpSelectionMerge %34 None
OpSwitch %33 %36 1 %35
%35 = OpLabel
OpStore %i %int_0
OpBranch %38
%38 = OpLabel
OpLoopMerge %42 %41 None
OpBranch %39
%39 = OpLabel
%43 = OpLoad %int %i
%45 = OpSLessThan %bool %43 %int_10
OpBranchConditional %45 %40 %42
%40 = OpLabel
%47 = OpLoad %int %val
%48 = OpIAdd %int %47 %int_1
OpStore %val %48
OpBranch %41
%50 = OpLabel
%49 = OpLoad %int %val
%51 = OpIAdd %int %49 %int_1
OpStore %val %51
OpBranch %41
%41 = OpLabel
%52 = OpLoad %int %i
%53 = OpIAdd %int %52 %int_1
OpStore %i %53
OpBranch %38
%42 = OpLabel
OpBranch %36
%36 = OpLabel
%54 = OpLoad %int %val
%55 = OpIAdd %int %54 %int_1
OpStore %val %55
OpBranch %34
%34 = OpLabel
%56 = OpLoad %int %val
%58 = OpIEqual %bool %56 %int_11
OpReturnValue %58
OpFunctionEnd
%loop_with_break_in_switch_bi = OpFunction %bool None %27
%59 = OpFunctionParameter %_ptr_Function_int
%60 = OpLabel
%val_0 = OpVariable %_ptr_Function_int Function
%i_0 = OpVariable %_ptr_Function_int Function
OpStore %val_0 %int_0
OpStore %i_0 %int_0
OpBranch %63
%63 = OpLabel
OpLoopMerge %67 %66 None
OpBranch %64
%64 = OpLabel
%68 = OpLoad %int %i_0
%69 = OpSLessThan %bool %68 %int_10
OpBranchConditional %69 %65 %67
%65 = OpLabel
%70 = OpLoad %int %59
OpSelectionMerge %71 None
OpSwitch %70 %73 1 %72
%72 = OpLabel
%74 = OpLoad %int %val_0
%75 = OpIAdd %int %74 %int_1
OpStore %val_0 %75
OpBranch %71
%73 = OpLabel
OpReturnValue %false
%71 = OpLabel
%77 = OpLoad %int %val_0
%78 = OpIAdd %int %77 %int_1
OpStore %val_0 %78
OpBranch %66
%66 = OpLabel
%79 = OpLoad %int %i_0
%80 = OpIAdd %int %79 %int_1
OpStore %i_0 %80
OpBranch %63
%67 = OpLabel
%81 = OpLoad %int %val_0
%83 = OpIEqual %bool %81 %int_20
OpReturnValue %83
OpFunctionEnd
%loop_with_continue_in_switch_bi = OpFunction %bool None %27
%84 = OpFunctionParameter %_ptr_Function_int
%85 = OpLabel
%val_1 = OpVariable %_ptr_Function_int Function
%i_1 = OpVariable %_ptr_Function_int Function
OpStore %val_1 %int_0
OpStore %i_1 %int_0
OpBranch %88
%88 = OpLabel
OpLoopMerge %92 %91 None
OpBranch %89
%89 = OpLabel
%93 = OpLoad %int %i_1
%94 = OpSLessThan %bool %93 %int_10
OpBranchConditional %94 %90 %92
%90 = OpLabel
%95 = OpLoad %int %84
OpSelectionMerge %96 None
OpSwitch %95 %98 1 %97
%97 = OpLabel
%99 = OpLoad %int %val_1
%100 = OpIAdd %int %99 %int_1
OpStore %val_1 %100
OpBranch %91
%98 = OpLabel
OpReturnValue %false
%96 = OpLabel
%101 = OpLoad %int %val_1
%102 = OpIAdd %int %101 %int_1
OpStore %val_1 %102
OpBranch %91
%91 = OpLabel
%103 = OpLoad %int %i_1
%104 = OpIAdd %int %103 %int_1
OpStore %i_1 %104
OpBranch %88
%92 = OpLabel
%105 = OpLoad %int %val_1
%106 = OpIEqual %bool %105 %int_10
OpReturnValue %106
OpFunctionEnd
%main = OpFunction %v4float None %107
%108 = OpFunctionParameter %_ptr_Function_v2float
%109 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%_0_val = OpVariable %_ptr_Function_int Function
%_1_i = OpVariable %_ptr_Function_int Function
%144 = OpVariable %_ptr_Function_int Function
%150 = OpVariable %_ptr_Function_int Function
%156 = OpVariable %_ptr_Function_int Function
%159 = OpVariable %_ptr_Function_v4float Function
%111 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%113 = OpLoad %v4float %111
%114 = OpCompositeExtract %float %113 1
%115 = OpConvertFToS %int %114
OpStore %x %115
OpStore %_0_val %int_0
%117 = OpLoad %int %x
OpSelectionMerge %118 None
OpSwitch %117 %120 1 %119
%119 = OpLabel
OpStore %_1_i %int_0
OpBranch %122
%122 = OpLabel
OpLoopMerge %126 %125 None
OpBranch %123
%123 = OpLabel
%127 = OpLoad %int %_1_i
%128 = OpSLessThan %bool %127 %int_10
OpBranchConditional %128 %124 %126
%124 = OpLabel
%129 = OpLoad %int %_0_val
%130 = OpIAdd %int %129 %int_1
OpStore %_0_val %130
OpBranch %126
%132 = OpLabel
%131 = OpLoad %int %_0_val
%133 = OpIAdd %int %131 %int_1
OpStore %_0_val %133
OpBranch %125
%125 = OpLabel
%134 = OpLoad %int %_1_i
%135 = OpIAdd %int %134 %int_1
OpStore %_1_i %135
OpBranch %122
%126 = OpLabel
OpBranch %120
%120 = OpLabel
%136 = OpLoad %int %_0_val
%137 = OpIAdd %int %136 %int_1
OpStore %_0_val %137
OpBranch %118
%118 = OpLabel
%138 = OpLoad %int %_0_val
%140 = OpIEqual %bool %138 %int_2
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%143 = OpLoad %int %x
OpStore %144 %143
%145 = OpFunctionCall %bool %switch_with_continue_in_loop_bi %144
OpBranch %142
%142 = OpLabel
%146 = OpPhi %bool %false %118 %145 %141
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpLoad %int %x
OpStore %150 %149
%151 = OpFunctionCall %bool %loop_with_break_in_switch_bi %150
OpBranch %148
%148 = OpLabel
%152 = OpPhi %bool %false %142 %151 %147
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpLoad %int %x
OpStore %156 %155
%157 = OpFunctionCall %bool %loop_with_continue_in_switch_bi %156
OpBranch %154
%154 = OpLabel
%158 = OpPhi %bool %false %148 %157 %153
OpSelectionMerge %163 None
OpBranchConditional %158 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%165 = OpLoad %v4float %164
OpStore %159 %165
OpBranch %163
%162 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%167 = OpLoad %v4float %166
OpStore %159 %167
OpBranch %163
%163 = OpLabel
%168 = OpLoad %v4float %159
OpReturnValue %168
OpFunctionEnd
