OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %b "b"
OpName %c "c"
OpName %x "x"
OpName %d "d"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
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
OpStore %x %int_0
OpBranch %32
%32 = OpLabel
OpLoopMerge %36 %35 None
OpBranch %33
%33 = OpLabel
%37 = OpLoad %int %x
%39 = OpSLessThan %bool %37 %int_1
OpBranchConditional %39 %34 %36
%34 = OpLabel
%40 = OpLoad %float %c
%42 = OpFSub %float %40 %float_1
OpStore %b %42
OpBranch %36
%35 = OpLabel
%43 = OpLoad %int %x
%44 = OpIAdd %int %43 %int_1
OpStore %x %44
OpBranch %32
%36 = OpLabel
%46 = OpLoad %float %c
OpStore %d %46
%47 = OpLoad %float %b
%48 = OpFAdd %float %47 %float_1
OpStore %b %48
%49 = OpLoad %float %d
%50 = OpFAdd %float %49 %float_1
OpStore %d %50
%51 = OpLoad %float %b
%52 = OpFOrdEqual %bool %51 %float_2
%53 = OpSelect %float %52 %float_1 %float_0
%54 = OpLoad %float %b
%55 = OpFOrdEqual %bool %54 %float_3
%56 = OpSelect %float %55 %float_1 %float_0
%57 = OpLoad %float %d
%59 = OpFOrdEqual %bool %57 %float_5
%60 = OpSelect %float %59 %float_1 %float_0
%61 = OpLoad %float %d
%63 = OpFOrdEqual %bool %61 %float_4
%64 = OpSelect %float %63 %float_1 %float_0
%65 = OpCompositeConstruct %v4float %53 %56 %60 %64
OpReturnValue %65
OpFunctionEnd
