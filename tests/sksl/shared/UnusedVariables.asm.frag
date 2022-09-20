OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_entrypoint_v "_entrypoint_v"
OpName %userfunc_ff "userfunc_ff"
OpName %main "main"
OpName %b "b"
OpName %c "c"
OpName %x "x"
OpName %d "d"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%13 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%22 = OpTypeFunction %float %_ptr_Function_float
%float_1 = OpConstant %float 1
%28 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_77 = OpConstant %float 77
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_5 = OpConstant %float 5
%float_4 = OpConstant %float 4
%_entrypoint_v = OpFunction %void None %13
%14 = OpLabel
%18 = OpVariable %_ptr_Function_v2float Function
OpStore %18 %17
%20 = OpFunctionCall %v4float %main %18
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%userfunc_ff = OpFunction %float None %22
%23 = OpFunctionParameter %_ptr_Function_float
%24 = OpLabel
%25 = OpLoad %float %23
%27 = OpFAdd %float %25 %float_1
OpReturnValue %27
OpFunctionEnd
%main = OpFunction %v4float None %28
%29 = OpFunctionParameter %_ptr_Function_v2float
%30 = OpLabel
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%43 = OpVariable %_ptr_Function_float Function
%46 = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_float Function
OpStore %b %float_2
OpStore %c %float_3
%36 = OpFAdd %float %float_3 %float_77
%37 = OpFAdd %float %float_3 %float_77
OpStore %b %37
%39 = OpFAdd %float %float_3 %float_77
%38 = OpExtInst %float %1 Sin %39
%41 = OpFAdd %float %float_3 %float_77
%40 = OpExtInst %float %1 Sin %41
OpStore %b %40
%42 = OpFAdd %float %float_3 %float_77
OpStore %43 %42
%44 = OpFunctionCall %float %userfunc_ff %43
%45 = OpFAdd %float %float_3 %float_77
OpStore %46 %45
%47 = OpFunctionCall %float %userfunc_ff %46
OpStore %b %47
OpStore %x %int_0
OpBranch %52
%52 = OpLabel
OpLoopMerge %56 %55 None
OpBranch %53
%53 = OpLabel
%57 = OpLoad %int %x
%59 = OpSLessThan %bool %57 %int_1
OpBranchConditional %59 %54 %56
%54 = OpLabel
OpBranch %55
%55 = OpLabel
%60 = OpLoad %int %x
%61 = OpIAdd %int %60 %int_1
OpStore %x %61
OpBranch %52
%56 = OpLabel
%63 = OpLoad %float %c
OpStore %d %63
%64 = OpLoad %float %b
%65 = OpFAdd %float %64 %float_1
OpStore %b %65
%66 = OpFAdd %float %63 %float_1
OpStore %d %66
%67 = OpFOrdEqual %bool %65 %float_2
%68 = OpSelect %float %67 %float_1 %float_0
%69 = OpFOrdEqual %bool %65 %float_3
%70 = OpSelect %float %69 %float_1 %float_0
%72 = OpFOrdEqual %bool %66 %float_5
%73 = OpSelect %float %72 %float_1 %float_0
%75 = OpFOrdEqual %bool %66 %float_4
%76 = OpSelect %float %75 %float_1 %float_0
%77 = OpCompositeConstruct %v4float %68 %70 %73 %76
OpReturnValue %77
OpFunctionEnd
