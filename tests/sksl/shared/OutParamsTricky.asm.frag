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
OpName %tricky_h2hhh2h "tricky_h2hhh2h"
OpName %func_vh4 "func_vh4"
OpName %t "t"
OpName %main "main"
OpName %result "result"
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
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %t RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
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
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%27 = OpTypeFunction %v2float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_v2float %_ptr_Function_float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%41 = OpTypeFunction %void %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_5 = OpConstant %float 5
%62 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%67 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%70 = OpConstantComposite %v4float %float_2 %float_3 %float_0 %float_5
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%tricky_h2hhh2h = OpFunction %v2float None %27
%29 = OpFunctionParameter %_ptr_Function_float
%30 = OpFunctionParameter %_ptr_Function_float
%31 = OpFunctionParameter %_ptr_Function_v2float
%32 = OpFunctionParameter %_ptr_Function_float
%33 = OpLabel
%34 = OpLoad %v2float %31
%35 = OpVectorShuffle %v2float %34 %34 1 0
OpStore %31 %35
%36 = OpLoad %float %29
%37 = OpLoad %float %30
%38 = OpFAdd %float %36 %37
%39 = OpLoad %float %32
%40 = OpCompositeConstruct %v2float %38 %39
OpReturnValue %40
OpFunctionEnd
%func_vh4 = OpFunction %void None %41
%43 = OpFunctionParameter %_ptr_Function_v4float
%44 = OpLabel
%t = OpVariable %_ptr_Function_v2float Function
%47 = OpVariable %_ptr_Function_float Function
%49 = OpVariable %_ptr_Function_float Function
%52 = OpVariable %_ptr_Function_v2float Function
%54 = OpVariable %_ptr_Function_float Function
OpStore %47 %float_1
OpStore %49 %float_2
%50 = OpLoad %v4float %43
%51 = OpVectorShuffle %v2float %50 %50 0 2
OpStore %52 %51
OpStore %54 %float_5
%55 = OpFunctionCall %v2float %tricky_h2hhh2h %47 %49 %52 %54
%56 = OpLoad %v2float %52
%57 = OpLoad %v4float %43
%58 = OpVectorShuffle %v4float %57 %56 4 1 5 3
OpStore %43 %58
OpStore %t %55
%59 = OpLoad %v2float %t
%60 = OpLoad %v4float %43
%61 = OpVectorShuffle %v4float %60 %59 0 4 2 5
OpStore %43 %61
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %62
%63 = OpFunctionParameter %_ptr_Function_v2float
%64 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%74 = OpVariable %_ptr_Function_v4float Function
OpStore %result %67
%68 = OpFunctionCall %void %func_vh4 %result
%69 = OpLoad %v4float %result
%71 = OpFOrdEqual %v4bool %69 %70
%73 = OpAll %bool %71
OpSelectionMerge %77 None
OpBranchConditional %73 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%82 = OpLoad %v4float %78
OpStore %74 %82
OpBranch %77
%76 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %14 %int_1
%85 = OpLoad %v4float %83
OpStore %74 %85
OpBranch %77
%77 = OpLabel
%86 = OpLoad %v4float %74
OpReturnValue %86
OpFunctionEnd
