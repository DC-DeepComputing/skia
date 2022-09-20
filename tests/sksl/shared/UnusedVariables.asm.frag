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
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %x "x"
OpName %d "d"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
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
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%44 = OpVariable %_ptr_Function_float Function
%47 = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_float Function
OpStore %a %float_1
OpStore %b %float_2
OpStore %c %float_3
%37 = OpFAdd %float %float_3 %float_77
OpStore %a %37
%38 = OpFAdd %float %float_3 %float_77
OpStore %b %38
%40 = OpFAdd %float %float_3 %float_77
%39 = OpExtInst %float %1 Sin %40
OpStore %a %39
%42 = OpFAdd %float %float_3 %float_77
%41 = OpExtInst %float %1 Sin %42
OpStore %b %41
%43 = OpFAdd %float %float_3 %float_77
OpStore %44 %43
%45 = OpFunctionCall %float %userfunc_ff %44
OpStore %a %45
%46 = OpFAdd %float %float_3 %float_77
OpStore %47 %46
%48 = OpFunctionCall %float %userfunc_ff %47
OpStore %b %48
OpStore %x %int_0
OpBranch %53
%53 = OpLabel
OpLoopMerge %57 %56 None
OpBranch %54
%54 = OpLabel
%58 = OpLoad %int %x
%60 = OpSLessThan %bool %58 %int_1
OpBranchConditional %60 %55 %57
%55 = OpLabel
OpBranch %56
%56 = OpLabel
%61 = OpLoad %int %x
%62 = OpIAdd %int %61 %int_1
OpStore %x %62
OpBranch %53
%57 = OpLabel
%64 = OpLoad %float %c
OpStore %d %64
%65 = OpLoad %float %b
%66 = OpFAdd %float %65 %float_1
OpStore %b %66
%67 = OpFAdd %float %64 %float_1
OpStore %d %67
%68 = OpFOrdEqual %bool %66 %float_2
%69 = OpSelect %float %68 %float_1 %float_0
%70 = OpFOrdEqual %bool %66 %float_3
%71 = OpSelect %float %70 %float_1 %float_0
%73 = OpFOrdEqual %bool %67 %float_5
%74 = OpSelect %float %73 %float_1 %float_0
%76 = OpFOrdEqual %bool %67 %float_4
%77 = OpSelect %float %76 %float_1 %float_0
%78 = OpCompositeConstruct %v4float %69 %71 %74 %77
OpReturnValue %78
OpFunctionEnd
