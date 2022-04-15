OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
OpName %exponents "exponents"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %exponents RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
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
%float_n1_5625 = OpConstant %float -1.5625
%float_0_75 = OpConstant %float 0.75
%float_3_375 = OpConstant %float 3.375
%31 = OpConstantComposite %v4float %float_n1_5625 %float_0 %float_0_75 %float_3_375
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%37 = OpConstantComposite %v4float %float_2 %float_3 %float_1 %float_1_5
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%53 = OpConstantComposite %v2float %float_2 %float_3
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%67 = OpConstantComposite %v3float %float_2 %float_3 %float_1
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1_5625 = OpConstant %float 1.5625
%94 = OpConstantComposite %v2float %float_1_5625 %float_0
%102 = OpConstantComposite %v3float %float_1_5625 %float_0 %float_0_75
%110 = OpConstantComposite %v4float %float_1_5625 %float_0 %float_0_75 %float_3_375
%int_1 = OpConstant %int 1
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
%expected = OpVariable %_ptr_Function_v4float Function
%exponents = OpVariable %_ptr_Function_v4float Function
%115 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
OpStore %exponents %37
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %40
%45 = OpCompositeExtract %float %44 0
%39 = OpExtInst %float %1 Pow %45 %float_2
%46 = OpFOrdEqual %bool %39 %float_n1_5625
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
%49 = OpExtInst %v2float %1 Pow %52 %53
%54 = OpLoad %v4float %expected
%55 = OpVectorShuffle %v2float %54 %54 0 1
%56 = OpFOrdEqual %v2bool %49 %55
%58 = OpAll %bool %56
OpBranch %48
%48 = OpLabel
%59 = OpPhi %bool %false %25 %58 %47
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v3float %64 %64 0 1 2
%62 = OpExtInst %v3float %1 Pow %65 %67
%68 = OpLoad %v4float %expected
%69 = OpVectorShuffle %v3float %68 %68 0 1 2
%70 = OpFOrdEqual %v3bool %62 %69
%72 = OpAll %bool %70
OpBranch %61
%61 = OpLabel
%73 = OpPhi %bool %false %48 %72 %60
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %77
%79 = OpLoad %v4float %exponents
%76 = OpExtInst %v4float %1 Pow %78 %79
%80 = OpLoad %v4float %expected
%81 = OpFOrdEqual %v4bool %76 %80
%83 = OpAll %bool %81
OpBranch %75
%75 = OpLabel
%84 = OpPhi %bool %false %61 %83 %74
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%88 = OpLoad %v4float %expected
%89 = OpCompositeExtract %float %88 0
%90 = OpFOrdEqual %bool %float_1_5625 %89
OpBranch %86
%86 = OpLabel
%91 = OpPhi %bool %false %75 %90 %85
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%95 = OpLoad %v4float %expected
%96 = OpVectorShuffle %v2float %95 %95 0 1
%97 = OpFOrdEqual %v2bool %94 %96
%98 = OpAll %bool %97
OpBranch %93
%93 = OpLabel
%99 = OpPhi %bool %false %86 %98 %92
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%103 = OpLoad %v4float %expected
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%105 = OpFOrdEqual %v3bool %102 %104
%106 = OpAll %bool %105
OpBranch %101
%101 = OpLabel
%107 = OpPhi %bool %false %93 %106 %100
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpLoad %v4float %expected
%112 = OpFOrdEqual %v4bool %110 %111
%113 = OpAll %bool %112
OpBranch %109
%109 = OpLabel
%114 = OpPhi %bool %false %101 %113 %108
OpSelectionMerge %118 None
OpBranchConditional %114 %116 %117
%116 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%121 = OpLoad %v4float %119
OpStore %115 %121
OpBranch %118
%117 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%124 = OpLoad %v4float %122
OpStore %115 %124
OpBranch %118
%118 = OpLabel
%125 = OpLoad %v4float %115
OpReturnValue %125
OpFunctionEnd
