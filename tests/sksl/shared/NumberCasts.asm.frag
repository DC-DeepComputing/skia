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
OpName %B "B"
OpName %F "F"
OpName %I "I"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
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
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%true = OpConstantTrue %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_1_23000002 = OpConstant %float 1.23000002
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_int = OpTypePointer Function %int
%false = OpConstantFalse %bool
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
%B = OpVariable %_ptr_Function_v3bool Function
%F = OpVariable %_ptr_Function_v3float Function
%I = OpVariable %_ptr_Function_v3int Function
%29 = OpAccessChain %_ptr_Function_bool %B %int_0
OpStore %29 %true
%33 = OpAccessChain %_ptr_Function_bool %B %int_1
OpStore %33 %true
%35 = OpAccessChain %_ptr_Function_bool %B %int_2
OpStore %35 %true
%41 = OpAccessChain %_ptr_Function_float %F %int_0
OpStore %41 %float_1_23000002
%43 = OpAccessChain %_ptr_Function_float %F %int_1
OpStore %43 %float_0
%45 = OpAccessChain %_ptr_Function_float %F %int_2
OpStore %45 %float_1
%49 = OpAccessChain %_ptr_Function_int %I %int_0
OpStore %49 %int_1
%51 = OpAccessChain %_ptr_Function_int %I %int_1
OpStore %51 %int_1
%52 = OpAccessChain %_ptr_Function_int %I %int_2
OpStore %52 %int_1
%53 = OpLoad %v3float %F
%54 = OpCompositeExtract %float %53 0
%55 = OpLoad %v3float %F
%56 = OpCompositeExtract %float %55 1
%57 = OpFMul %float %54 %56
%58 = OpLoad %v3float %F
%59 = OpCompositeExtract %float %58 2
%60 = OpFMul %float %57 %59
%62 = OpLoad %v3bool %B
%63 = OpCompositeExtract %bool %62 0
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %v3bool %B
%67 = OpCompositeExtract %bool %66 1
OpBranch %65
%65 = OpLabel
%68 = OpPhi %bool %false %24 %67 %64
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %v3bool %B
%72 = OpCompositeExtract %bool %71 2
OpBranch %70
%70 = OpLabel
%73 = OpPhi %bool %false %65 %72 %69
%74 = OpSelect %float %73 %float_1 %float_0
%75 = OpLoad %v3int %I
%76 = OpCompositeExtract %int %75 0
%77 = OpLoad %v3int %I
%78 = OpCompositeExtract %int %77 1
%79 = OpIMul %int %76 %78
%80 = OpLoad %v3int %I
%81 = OpCompositeExtract %int %80 2
%82 = OpIMul %int %79 %81
%83 = OpConvertSToF %float %82
%84 = OpCompositeConstruct %v4float %60 %74 %float_0 %83
OpReturnValue %84
OpFunctionEnd
