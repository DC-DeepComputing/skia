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
OpName %main "main"
OpName %i2 "i2"
OpName %s2 "s2"
OpName %f2 "f2"
OpName %h2 "h2"
OpName %cf2 "cf2"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_int_int_2 ArrayStride 16
OpDecorate %s2 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %h2 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
%int_1 = OpConstant %int 1
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%i2 = OpVariable %_ptr_Function__arr_int_int_2 Function
%s2 = OpVariable %_ptr_Function__arr_int_int_2 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%h2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%cf2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%81 = OpVariable %_ptr_Function_v4float Function
%32 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %i2 %32
%34 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %s2 %34
%40 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %f2 %40
%42 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %h2 %42
%43 = OpLoad %_arr_int_int_2 %s2
OpStore %i2 %43
OpStore %s2 %43
%44 = OpLoad %_arr_float_int_2 %h2
OpStore %f2 %44
OpStore %h2 %44
OpStore %cf2 %40
%47 = OpCompositeExtract %int %43 0
%48 = OpIEqual %bool %47 %47
%49 = OpCompositeExtract %int %43 1
%50 = OpIEqual %bool %49 %49
%51 = OpLogicalAnd %bool %50 %48
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%54 = OpCompositeExtract %float %44 0
%55 = OpFOrdEqual %bool %54 %54
%56 = OpCompositeExtract %float %44 1
%57 = OpFOrdEqual %bool %56 %56
%58 = OpLogicalAnd %bool %57 %55
OpBranch %53
%53 = OpLabel
%59 = OpPhi %bool %false %25 %58 %52
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %_arr_int_int_2 %i2
%63 = OpCompositeExtract %int %62 0
%64 = OpIEqual %bool %63 %int_1
%65 = OpCompositeExtract %int %62 1
%66 = OpIEqual %bool %65 %int_2
%67 = OpLogicalAnd %bool %66 %64
OpBranch %61
%61 = OpLabel
%68 = OpPhi %bool %false %53 %67 %60
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%71 = OpLoad %_arr_float_int_2 %h2
%72 = OpLoad %_arr_float_int_2 %cf2
%73 = OpCompositeExtract %float %71 0
%74 = OpCompositeExtract %float %72 0
%75 = OpFOrdEqual %bool %73 %74
%76 = OpCompositeExtract %float %71 1
%77 = OpCompositeExtract %float %72 1
%78 = OpFOrdEqual %bool %76 %77
%79 = OpLogicalAnd %bool %78 %75
OpBranch %70
%70 = OpLabel
%80 = OpPhi %bool %false %61 %79 %69
OpSelectionMerge %85 None
OpBranchConditional %80 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%89 = OpLoad %v4float %86
OpStore %81 %89
OpBranch %85
%84 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%91 = OpLoad %v4float %90
OpStore %81 %91
OpBranch %85
%85 = OpLabel
%92 = OpLoad %v4float %81
OpReturnValue %92
OpFunctionEnd
