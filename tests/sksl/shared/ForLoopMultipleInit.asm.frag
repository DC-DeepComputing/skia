OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %result "result"
OpName %a "a"
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
OpDecorate %result RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %120 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%14 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%22 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%27 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%float_10 = OpConstant %float 10
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%_ptr_Function_int = OpTypePointer Function %int
%int_10 = OpConstant %int 10
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_9 = OpConstant %float 9
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %14
%15 = OpLabel
%19 = OpVariable %_ptr_Function_v2float Function
OpStore %19 %18
%21 = OpFunctionCall %v4float %main %19
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %22
%23 = OpFunctionParameter %_ptr_Function_v2float
%24 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function__arr_float_int_2 Function
%e = OpVariable %_ptr_Function__arr_float_int_4 Function
%f = OpVariable %_ptr_Function_float Function
OpStore %result %27
OpStore %a %float_0
OpStore %b %float_0
OpBranch %31
%31 = OpLabel
OpLoopMerge %35 %34 None
OpBranch %32
%32 = OpLabel
%37 = OpLoad %float %a
%39 = OpFOrdLessThan %bool %37 %float_10
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %float %b
%43 = OpFOrdLessThan %bool %42 %float_10
OpBranch %41
%41 = OpLabel
%44 = OpPhi %bool %false %32 %43 %40
OpBranchConditional %44 %33 %35
%33 = OpLabel
%45 = OpAccessChain %_ptr_Function_float %result %int_0
%48 = OpLoad %float %45
%49 = OpLoad %float %a
%50 = OpFAdd %float %48 %49
OpStore %45 %50
%51 = OpAccessChain %_ptr_Function_float %result %int_1
%53 = OpLoad %float %51
%54 = OpLoad %float %b
%55 = OpFAdd %float %53 %54
OpStore %51 %55
OpBranch %34
%34 = OpLabel
%57 = OpLoad %float %a
%58 = OpFAdd %float %57 %float_1
OpStore %a %58
%59 = OpLoad %float %b
%60 = OpFAdd %float %59 %float_1
OpStore %b %60
OpBranch %31
%35 = OpLabel
OpStore %c %int_0
OpBranch %63
%63 = OpLabel
OpLoopMerge %67 %66 None
OpBranch %64
%64 = OpLabel
%68 = OpLoad %int %c
%70 = OpSLessThan %bool %68 %int_10
OpBranchConditional %70 %65 %67
%65 = OpLabel
%71 = OpAccessChain %_ptr_Function_float %result %int_2
%73 = OpLoad %float %71
%74 = OpFAdd %float %73 %float_1
OpStore %71 %74
OpBranch %66
%66 = OpLabel
%75 = OpLoad %int %c
%76 = OpIAdd %int %75 %int_1
OpStore %c %76
OpBranch %63
%67 = OpLabel
%80 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_10
OpStore %d %80
%88 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %e %88
OpStore %f %float_9
OpBranch %91
%91 = OpLabel
OpLoopMerge %95 %94 None
OpBranch %92
%92 = OpLabel
%96 = OpAccessChain %_ptr_Function_float %d %int_0
%97 = OpLoad %float %96
%98 = OpAccessChain %_ptr_Function_float %d %int_1
%99 = OpLoad %float %98
%100 = OpFOrdLessThan %bool %97 %99
OpBranchConditional %100 %93 %95
%93 = OpLabel
%101 = OpAccessChain %_ptr_Function_float %e %int_0
%102 = OpLoad %float %101
%103 = OpLoad %float %f
%104 = OpFMul %float %102 %103
%105 = OpAccessChain %_ptr_Function_float %result %int_3
OpStore %105 %104
OpBranch %94
%94 = OpLabel
%107 = OpAccessChain %_ptr_Function_float %d %int_0
%108 = OpLoad %float %107
%109 = OpFAdd %float %108 %float_1
OpStore %107 %109
OpBranch %91
%95 = OpLabel
OpBranch %110
%110 = OpLabel
OpLoopMerge %114 %113 None
OpBranch %111
%111 = OpLabel
OpBranch %112
%112 = OpLabel
OpBranch %114
%113 = OpLabel
OpBranch %110
%114 = OpLabel
OpBranch %115
%115 = OpLabel
OpLoopMerge %119 %118 None
OpBranch %116
%116 = OpLabel
OpBranch %117
%117 = OpLabel
OpBranch %119
%118 = OpLabel
OpBranch %115
%119 = OpLabel
%120 = OpLoad %v4float %result
OpReturnValue %120
OpFunctionEnd
