### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %63 = OpExtInst %v2float %1 Modf %65 %4294967295

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
OpName %value "value"
OpName %whole "whole"
OpName %fraction "fraction"
OpName %ok "ok"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_2_5 = OpConstant %float 2.5
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%float_2 = OpConstant %float 2
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_bool = OpTypePointer Function %bool
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%value = OpVariable %_ptr_Function_v4float Function
%whole = OpVariable %_ptr_Function_v4float Function
%fraction = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%111 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
%35 = OpVectorShuffle %v4float %34 %34 1 1 1 1
%37 = OpVectorTimesScalar %v4float %35 %float_2_5
OpStore %value %37
%44 = OpLoad %v4float %value
%45 = OpCompositeExtract %float %44 0
%46 = OpAccessChain %_ptr_Function_float %whole %int_0
%43 = OpExtInst %float %1 Modf %45 %46
%48 = OpAccessChain %_ptr_Function_float %fraction %int_0
OpStore %48 %43
%50 = OpLoad %v4float %whole
%51 = OpCompositeExtract %float %50 0
%53 = OpFOrdEqual %bool %51 %float_2
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%56 = OpLoad %v4float %fraction
%57 = OpCompositeExtract %float %56 0
%59 = OpFOrdEqual %bool %57 %float_0_5
OpBranch %55
%55 = OpLabel
%60 = OpPhi %bool %false %27 %59 %54
%61 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %61 %60
%64 = OpLoad %v4float %value
%65 = OpVectorShuffle %v2float %64 %64 0 1
%63 = OpExtInst %v2float %1 Modf %65 %4294967295
%66 = OpLoad %v4float %fraction
%67 = OpVectorShuffle %v4float %66 %63 4 5 2 3
OpStore %fraction %67
%68 = OpLoad %v4float %whole
%69 = OpCompositeExtract %float %68 1
%70 = OpFOrdEqual %bool %69 %float_2
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpLoad %v4float %fraction
%74 = OpCompositeExtract %float %73 1
%75 = OpFOrdEqual %bool %74 %float_0_5
OpBranch %72
%72 = OpLabel
%76 = OpPhi %bool %false %55 %75 %71
%77 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %77 %76
%80 = OpLoad %v4float %value
%81 = OpVectorShuffle %v3float %80 %80 0 1 2
%79 = OpExtInst %v3float %1 Modf %81 %4294967295
%83 = OpLoad %v4float %fraction
%84 = OpVectorShuffle %v4float %83 %79 4 5 6 3
OpStore %fraction %84
%85 = OpLoad %v4float %whole
%86 = OpCompositeExtract %float %85 2
%87 = OpFOrdEqual %bool %86 %float_2
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %v4float %fraction
%91 = OpCompositeExtract %float %90 2
%92 = OpFOrdEqual %bool %91 %float_0_5
OpBranch %89
%89 = OpLabel
%93 = OpPhi %bool %false %72 %92 %88
%94 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %94 %93
%97 = OpLoad %v4float %value
%96 = OpExtInst %v4float %1 Modf %97 %whole
OpStore %fraction %96
%98 = OpLoad %v4float %whole
%99 = OpCompositeExtract %float %98 3
%100 = OpFOrdEqual %bool %99 %float_2
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpLoad %v4float %fraction
%104 = OpCompositeExtract %float %103 3
%105 = OpFOrdEqual %bool %104 %float_0_5
OpBranch %102
%102 = OpLabel
%106 = OpPhi %bool %false %89 %105 %101
%107 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %107 %106
%110 = OpLoad %v4bool %ok
%109 = OpAll %bool %110
OpSelectionMerge %114 None
OpBranchConditional %109 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%116 = OpLoad %v4float %115
OpStore %111 %116
OpBranch %114
%113 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%118 = OpLoad %v4float %117
OpStore %111 %118
OpBranch %114
%114 = OpLabel
%119 = OpLoad %v4float %111
OpReturnValue %119
OpFunctionEnd

1 error
