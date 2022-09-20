OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_entrypoint_v "_entrypoint_v"
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
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_77 = OpConstant %float 77
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
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_float Function
%x = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function_float Function
OpStore %a %float_1
OpStore %b %float_2
OpStore %c %float_3
OpStore %a %float_77
OpStore %b %float_77
%31 = OpExtInst %float %1 Sin %float_3
OpStore %a %31
%32 = OpExtInst %float %1 Sin %float_3
OpStore %b %32
%33 = OpFAdd %float %float_3 %float_1
OpStore %a %33
%34 = OpFAdd %float %float_3 %float_1
OpStore %b %34
OpStore %x %int_0
OpBranch %39
%39 = OpLabel
OpLoopMerge %43 %42 None
OpBranch %40
%40 = OpLabel
%44 = OpLoad %int %x
%46 = OpSLessThan %bool %44 %int_1
OpBranchConditional %46 %41 %43
%41 = OpLabel
OpBranch %42
%42 = OpLabel
%47 = OpLoad %int %x
%48 = OpIAdd %int %47 %int_1
OpStore %x %48
OpBranch %39
%43 = OpLabel
%50 = OpLoad %float %c
OpStore %d %50
%51 = OpLoad %float %b
%52 = OpFAdd %float %51 %float_1
OpStore %b %52
%53 = OpFAdd %float %50 %float_1
OpStore %d %53
%54 = OpFOrdEqual %bool %52 %float_2
%55 = OpSelect %float %54 %float_1 %float_0
%56 = OpFOrdEqual %bool %52 %float_3
%57 = OpSelect %float %56 %float_1 %float_0
%59 = OpFOrdEqual %bool %53 %float_5
%60 = OpSelect %float %59 %float_1 %float_0
%62 = OpFOrdEqual %bool %53 %float_4
%63 = OpSelect %float %62 %float_1 %float_0
%64 = OpCompositeConstruct %v4float %55 %57 %60 %63
OpReturnValue %64
OpFunctionEnd
