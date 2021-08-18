OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "testMatrix3x3"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %testMatrix2x3 "testMatrix2x3"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 80
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 96
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat2v2float %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%28 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_1 = OpConstant %int 1
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%25 = OpVariable %_ptr_Function_v2float Function
OpStore %25 %24
%27 = OpFunctionCall %v4float %main %25
OpStore %sk_FragColor %27
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %28
%29 = OpFunctionParameter %_ptr_Function_v2float
%30 = OpLabel
%testMatrix2x3 = OpVariable %_ptr_Function_mat2v3float Function
%117 = OpVariable %_ptr_Function_v4float Function
%40 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%41 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%42 = OpCompositeConstruct %mat2v3float %40 %41
OpStore %testMatrix2x3 %42
%45 = OpAccessChain %_ptr_Uniform_mat2v2float %12 %int_0
%49 = OpLoad %mat2v2float %45
%44 = OpTranspose %mat2v2float %49
%50 = OpCompositeConstruct %v2float %float_1 %float_3
%51 = OpCompositeConstruct %v2float %float_2 %float_4
%52 = OpCompositeConstruct %mat2v2float %50 %51
%54 = OpCompositeExtract %v2float %44 0
%55 = OpCompositeExtract %v2float %52 0
%56 = OpFOrdEqual %v2bool %54 %55
%57 = OpAll %bool %56
%58 = OpCompositeExtract %v2float %44 1
%59 = OpCompositeExtract %v2float %52 1
%60 = OpFOrdEqual %v2bool %58 %59
%61 = OpAll %bool %60
%62 = OpLogicalAnd %bool %57 %61
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpLoad %mat2v3float %testMatrix2x3
%65 = OpTranspose %mat3v2float %66
%68 = OpCompositeConstruct %v2float %float_1 %float_4
%69 = OpCompositeConstruct %v2float %float_2 %float_5
%70 = OpCompositeConstruct %v2float %float_3 %float_6
%71 = OpCompositeConstruct %mat3v2float %68 %69 %70
%72 = OpCompositeExtract %v2float %65 0
%73 = OpCompositeExtract %v2float %71 0
%74 = OpFOrdEqual %v2bool %72 %73
%75 = OpAll %bool %74
%76 = OpCompositeExtract %v2float %65 1
%77 = OpCompositeExtract %v2float %71 1
%78 = OpFOrdEqual %v2bool %76 %77
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %75 %79
%81 = OpCompositeExtract %v2float %65 2
%82 = OpCompositeExtract %v2float %71 2
%83 = OpFOrdEqual %v2bool %81 %82
%84 = OpAll %bool %83
%85 = OpLogicalAnd %bool %80 %84
OpBranch %64
%64 = OpLabel
%86 = OpPhi %bool %false %30 %85 %63
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_mat3v3float %12 %int_1
%93 = OpLoad %mat3v3float %90
%89 = OpTranspose %mat3v3float %93
%97 = OpCompositeConstruct %v3float %float_1 %float_4 %float_7
%98 = OpCompositeConstruct %v3float %float_2 %float_5 %float_8
%99 = OpCompositeConstruct %v3float %float_3 %float_6 %float_9
%100 = OpCompositeConstruct %mat3v3float %97 %98 %99
%102 = OpCompositeExtract %v3float %89 0
%103 = OpCompositeExtract %v3float %100 0
%104 = OpFOrdEqual %v3bool %102 %103
%105 = OpAll %bool %104
%106 = OpCompositeExtract %v3float %89 1
%107 = OpCompositeExtract %v3float %100 1
%108 = OpFOrdEqual %v3bool %106 %107
%109 = OpAll %bool %108
%110 = OpLogicalAnd %bool %105 %109
%111 = OpCompositeExtract %v3float %89 2
%112 = OpCompositeExtract %v3float %100 2
%113 = OpFOrdEqual %v3bool %111 %112
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %110 %114
OpBranch %88
%88 = OpLabel
%116 = OpPhi %bool %false %64 %115 %87
OpSelectionMerge %121 None
OpBranchConditional %116 %119 %120
%119 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%125 = OpLoad %v4float %122
OpStore %117 %125
OpBranch %121
%120 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%128 = OpLoad %v4float %126
OpStore %117 %128
OpBranch %121
%121 = OpLabel
%129 = OpLoad %v4float %117
OpReturnValue %129
OpFunctionEnd
