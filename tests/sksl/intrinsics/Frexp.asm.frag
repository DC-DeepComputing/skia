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
OpName %value "value"
OpName %exp "exp"
OpName %result "result"
OpName %ok "ok"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
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
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%int_2 = OpConstant %int 2
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
%value = OpVariable %_ptr_Function_v4float Function
%exp = OpVariable %_ptr_Function_v4int Function
%result = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%47 = OpVariable %_ptr_Function_int Function
%67 = OpVariable %_ptr_Function_v2int Function
%87 = OpVariable %_ptr_Function_v3int Function
%105 = OpVariable %_ptr_Function_v4int Function
%117 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
%35 = OpVectorTimesScalar %v4float %33 %float_6
OpStore %value %35
%44 = OpCompositeExtract %float %35 0
%45 = OpAccessChain %_ptr_Function_int %exp %int_0
%43 = OpExtInst %float %1 Frexp %44 %47
%48 = OpLoad %int %47
OpStore %45 %48
%49 = OpAccessChain %_ptr_Function_float %result %int_0
OpStore %49 %43
%52 = OpLoad %v4float %result
%53 = OpCompositeExtract %float %52 0
%55 = OpFOrdEqual %bool %53 %float_0_75
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%58 = OpLoad %v4int %exp
%59 = OpCompositeExtract %int %58 0
%61 = OpIEqual %bool %59 %int_3
OpBranch %57
%57 = OpLabel
%62 = OpPhi %bool %false %25 %61 %56
%63 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %63 %62
%66 = OpVectorShuffle %v2float %35 %35 0 1
%65 = OpExtInst %v2float %1 Frexp %66 %67
%70 = OpLoad %v2int %67
%71 = OpLoad %v4int %exp
%72 = OpVectorShuffle %v4int %71 %70 4 5 2 3
OpStore %exp %72
%73 = OpLoad %v4float %result
%74 = OpVectorShuffle %v4float %73 %65 4 5 2 3
OpStore %result %74
%75 = OpCompositeExtract %float %74 1
%76 = OpFOrdEqual %bool %75 %float_0_75
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpCompositeExtract %int %72 1
%80 = OpIEqual %bool %79 %int_3
OpBranch %78
%78 = OpLabel
%81 = OpPhi %bool %false %57 %80 %77
%82 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %82 %81
%85 = OpVectorShuffle %v3float %35 %35 0 1 2
%84 = OpExtInst %v3float %1 Frexp %85 %87
%90 = OpLoad %v3int %87
%91 = OpLoad %v4int %exp
%92 = OpVectorShuffle %v4int %91 %90 4 5 6 3
OpStore %exp %92
%93 = OpLoad %v4float %result
%94 = OpVectorShuffle %v4float %93 %84 4 5 6 3
OpStore %result %94
%95 = OpCompositeExtract %float %94 2
%96 = OpFOrdEqual %bool %95 %float_0_75
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpCompositeExtract %int %92 2
%100 = OpIEqual %bool %99 %int_3
OpBranch %98
%98 = OpLabel
%101 = OpPhi %bool %false %78 %100 %97
%102 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %102 %101
%104 = OpExtInst %v4float %1 Frexp %35 %105
%106 = OpLoad %v4int %105
OpStore %exp %106
OpStore %result %104
%107 = OpCompositeExtract %float %104 3
%108 = OpFOrdEqual %bool %107 %float_0_75
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpCompositeExtract %int %106 3
%112 = OpIEqual %bool %111 %int_3
OpBranch %110
%110 = OpLabel
%113 = OpPhi %bool %false %98 %112 %109
%114 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %114 %113
%116 = OpLoad %v4bool %ok
%115 = OpAll %bool %116
OpSelectionMerge %120 None
OpBranchConditional %115 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%122 = OpLoad %v4float %121
OpStore %117 %122
OpBranch %120
%119 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%124 = OpLoad %v4float %123
OpStore %117 %124
OpBranch %120
%120 = OpLabel
%125 = OpLoad %v4float %117
OpReturnValue %125
OpFunctionEnd
