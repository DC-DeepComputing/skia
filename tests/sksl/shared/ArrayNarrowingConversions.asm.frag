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
OpName %main "main"
OpName %i2 "i2"
OpName %s2 "s2"
OpName %f2 "f2"
OpName %h2 "h2"
OpName %cf2 "cf2"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %_arr_int_int_2 ArrayStride 16
OpDecorate %s2 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %h2 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
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
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_arr_int_int_2 = OpTypeArray %int %int_2
%_ptr_Function__arr_int_int_2 = OpTypePointer Function %_arr_int_int_2
%int_1 = OpConstant %int 1
%_ptr_Function__arr_int_int_2_0 = OpTypePointer Function %_arr_int_int_2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %25
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpLabel
%i2 = OpVariable %_ptr_Function__arr_int_int_2 Function
%s2 = OpVariable %_ptr_Function__arr_int_int_2_0 Function
%f2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%h2 = OpVariable %_ptr_Function__arr_float_int_2_0 Function
%cf2 = OpVariable %_ptr_Function__arr_float_int_2 Function
%99 = OpVariable %_ptr_Function_v4float Function
%34 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %i2 %34
%37 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
OpStore %s2 %37
%43 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %f2 %43
%46 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %h2 %46
%47 = OpLoad %_arr_int_int_2 %s2
OpStore %i2 %47
%48 = OpLoad %_arr_int_int_2 %i2
OpStore %s2 %48
%49 = OpLoad %_arr_float_int_2 %h2
OpStore %f2 %49
%50 = OpLoad %_arr_float_int_2 %f2
OpStore %h2 %50
%52 = OpCompositeConstruct %_arr_float_int_2 %float_1 %float_2
OpStore %cf2 %52
%54 = OpLoad %_arr_int_int_2 %i2
%55 = OpLoad %_arr_int_int_2 %s2
%56 = OpCompositeExtract %int %54 0
%57 = OpCompositeExtract %int %55 0
%58 = OpIEqual %bool %56 %57
%59 = OpCompositeExtract %int %54 1
%60 = OpCompositeExtract %int %55 1
%61 = OpIEqual %bool %59 %60
%62 = OpLogicalAnd %bool %61 %58
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%65 = OpLoad %_arr_float_int_2 %f2
%66 = OpLoad %_arr_float_int_2 %h2
%67 = OpCompositeExtract %float %65 0
%68 = OpCompositeExtract %float %66 0
%69 = OpFOrdEqual %bool %67 %68
%70 = OpCompositeExtract %float %65 1
%71 = OpCompositeExtract %float %66 1
%72 = OpFOrdEqual %bool %70 %71
%73 = OpLogicalAnd %bool %72 %69
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %false %27 %73 %63
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %_arr_int_int_2 %i2
%78 = OpCompositeConstruct %_arr_int_int_2 %int_1 %int_2
%79 = OpCompositeExtract %int %77 0
%80 = OpCompositeExtract %int %78 0
%81 = OpIEqual %bool %79 %80
%82 = OpCompositeExtract %int %77 1
%83 = OpCompositeExtract %int %78 1
%84 = OpIEqual %bool %82 %83
%85 = OpLogicalAnd %bool %84 %81
OpBranch %76
%76 = OpLabel
%86 = OpPhi %bool %false %64 %85 %75
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %_arr_float_int_2 %h2
%90 = OpLoad %_arr_float_int_2 %cf2
%91 = OpCompositeExtract %float %89 0
%92 = OpCompositeExtract %float %90 0
%93 = OpFOrdEqual %bool %91 %92
%94 = OpCompositeExtract %float %89 1
%95 = OpCompositeExtract %float %90 1
%96 = OpFOrdEqual %bool %94 %95
%97 = OpLogicalAnd %bool %96 %93
OpBranch %88
%88 = OpLabel
%98 = OpPhi %bool %false %76 %97 %87
OpSelectionMerge %103 None
OpBranchConditional %98 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%107 = OpLoad %v4float %104
OpStore %99 %107
OpBranch %103
%102 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%109 = OpLoad %v4float %108
OpStore %99 %109
OpBranch %103
%103 = OpLabel
%110 = OpLoad %v4float %99
OpReturnValue %110
OpFunctionEnd
