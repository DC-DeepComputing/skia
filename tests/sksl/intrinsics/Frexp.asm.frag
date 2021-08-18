### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %66 = OpExtInst %v2float %1 Frexp %68 %4294967295

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
OpName %exp "exp"
OpName %result "result"
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
OpDecorate %112 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
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
%float_6 = OpConstant %float 6
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%float_0_75 = OpConstant %float 0.75
%int_3 = OpConstant %int 3
%_ptr_Function_bool = OpTypePointer Function %bool
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
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
%exp = OpVariable %_ptr_Function_v4int Function
%result = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%113 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
%35 = OpVectorShuffle %v4float %34 %34 1 1 1 1
%37 = OpVectorTimesScalar %v4float %35 %float_6
OpStore %value %37
%46 = OpLoad %v4float %value
%47 = OpCompositeExtract %float %46 0
%48 = OpAccessChain %_ptr_Function_int %exp %int_0
%45 = OpExtInst %float %1 Frexp %47 %48
%50 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %50 %45
%53 = OpLoad %v4float %result
%54 = OpCompositeExtract %float %53 0
%56 = OpFOrdEqual %bool %54 %float_0_75
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpLoad %v4int %exp
%60 = OpCompositeExtract %int %59 0
%62 = OpIEqual %bool %60 %int_3
OpBranch %58
%58 = OpLabel
%63 = OpPhi %bool %false %27 %62 %57
%64 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %64 %63
%67 = OpLoad %v4float %value
%68 = OpVectorShuffle %v2float %67 %67 0 1
%66 = OpExtInst %v2float %1 Frexp %68 %4294967295
%69 = OpLoad %v4float %result
%70 = OpVectorShuffle %v4float %69 %66 4 5 2 3
OpStore %result %70
%71 = OpLoad %v4float %result
%72 = OpCompositeExtract %float %71 1
%73 = OpFOrdEqual %bool %72 %float_0_75
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %v4int %exp
%77 = OpCompositeExtract %int %76 1
%78 = OpIEqual %bool %77 %int_3
OpBranch %75
%75 = OpLabel
%79 = OpPhi %bool %false %58 %78 %74
%80 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %80 %79
%83 = OpLoad %v4float %value
%84 = OpVectorShuffle %v3float %83 %83 0 1 2
%82 = OpExtInst %v3float %1 Frexp %84 %4294967295
%86 = OpLoad %v4float %result
%87 = OpVectorShuffle %v4float %86 %82 4 5 6 3
OpStore %result %87
%88 = OpLoad %v4float %result
%89 = OpCompositeExtract %float %88 2
%90 = OpFOrdEqual %bool %89 %float_0_75
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %v4int %exp
%94 = OpCompositeExtract %int %93 2
%95 = OpIEqual %bool %94 %int_3
OpBranch %92
%92 = OpLabel
%96 = OpPhi %bool %false %75 %95 %91
%97 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %97 %96
%100 = OpLoad %v4float %value
%99 = OpExtInst %v4float %1 Frexp %100 %exp
OpStore %result %99
%101 = OpLoad %v4float %result
%102 = OpCompositeExtract %float %101 3
%103 = OpFOrdEqual %bool %102 %float_0_75
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%106 = OpLoad %v4int %exp
%107 = OpCompositeExtract %int %106 3
%108 = OpIEqual %bool %107 %int_3
OpBranch %105
%105 = OpLabel
%109 = OpPhi %bool %false %92 %108 %104
%110 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %110 %109
%112 = OpLoad %v4bool %ok
%111 = OpAll %bool %112
OpSelectionMerge %116 None
OpBranchConditional %111 %114 %115
%114 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%118 = OpLoad %v4float %117
OpStore %113 %118
OpBranch %116
%115 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%120 = OpLoad %v4float %119
OpStore %113 %120
OpBranch %116
%116 = OpLabel
%121 = OpLoad %v4float %113
OpReturnValue %121
OpFunctionEnd

1 error
