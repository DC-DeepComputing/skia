OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %b "b"
OpName %c "c"
OpName %x "x"
OpName %d "d"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%12 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_77 = OpConstant %float 77
%float_1 = OpConstant %float 1
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_5 = OpConstant %float 5
%float_4 = OpConstant %float 4
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%17 = OpVariable %_ptr_Function_v2float Function
OpStore %17 %16
%19 = OpFunctionCall %v4float %main %17
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpFunctionParameter %_ptr_Function_v2float
%22 = OpLabel
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_float Function
OpStore %b %float_2
OpStore %c %float_3
OpStore %b %float_77
%29 = OpExtInst %float %1 Sin %float_3
%30 = OpExtInst %float %1 Sin %float_3
OpStore %b %30
%32 = OpFAdd %float %float_3 %float_1
%33 = OpFAdd %float %float_3 %float_1
OpStore %b %33
OpStore %x %int_0
OpBranch %38
%38 = OpLabel
OpLoopMerge %42 %41 None
OpBranch %39
%39 = OpLabel
%43 = OpLoad %int %x
%45 = OpSLessThan %bool %43 %int_1
OpBranchConditional %45 %40 %42
%40 = OpLabel
OpBranch %41
%41 = OpLabel
%46 = OpLoad %int %x
%47 = OpIAdd %int %46 %int_1
OpStore %x %47
OpBranch %38
%42 = OpLabel
%49 = OpLoad %float %c
OpStore %d %49
%50 = OpLoad %float %b
%51 = OpFAdd %float %50 %float_1
OpStore %b %51
%52 = OpFAdd %float %49 %float_1
OpStore %d %52
%53 = OpFOrdEqual %bool %51 %float_2
%54 = OpSelect %float %53 %float_1 %float_0
%55 = OpFOrdEqual %bool %51 %float_3
%56 = OpSelect %float %55 %float_1 %float_0
%58 = OpFOrdEqual %bool %52 %float_5
%59 = OpSelect %float %58 %float_1 %float_0
%61 = OpFOrdEqual %bool %52 %float_4
%62 = OpSelect %float %61 %float_1 %float_0
%63 = OpCompositeConstruct %v4float %54 %56 %59 %62
OpReturnValue %63
OpFunctionEnd
