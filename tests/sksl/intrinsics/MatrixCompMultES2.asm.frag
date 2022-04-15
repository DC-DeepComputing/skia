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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %h22 "h22"
OpName %f22 "f22"
OpName %h33 "h33"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %h22 RelaxedPrecision
OpDecorate %h33 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%34 = OpConstantComposite %v2float %float_0 %float_5
%35 = OpConstantComposite %v2float %float_10 %float_15
%36 = OpConstantComposite %mat2v2float %34 %35
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%45 = OpConstantComposite %v2float %float_1 %float_0
%46 = OpConstantComposite %v2float %float_0 %float_1
%47 = OpConstantComposite %mat2v2float %45 %46
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_2 = OpConstant %float 2
%61 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%62 = OpConstantComposite %mat3v3float %61 %61 %61
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_4 = OpConstant %float 4
%81 = OpConstantComposite %v2float %float_0 %float_4
%82 = OpConstantComposite %mat2v2float %45 %81
%float_6 = OpConstant %float 6
%float_8 = OpConstant %float 8
%float_12 = OpConstant %float 12
%float_14 = OpConstant %float 14
%float_16 = OpConstant %float 16
%float_18 = OpConstant %float 18
%100 = OpConstantComposite %v3float %float_2 %float_4 %float_6
%101 = OpConstantComposite %v3float %float_8 %float_10 %float_12
%102 = OpConstantComposite %v3float %float_14 %float_16 %float_18
%103 = OpConstantComposite %mat3v3float %100 %101 %102
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%h22 = OpVariable %_ptr_Function_mat2v2float Function
%f22 = OpVariable %_ptr_Function_mat2v2float Function
%h33 = OpVariable %_ptr_Function_mat3v3float Function
%117 = OpVariable %_ptr_Function_v4float Function
OpStore %h22 %36
%39 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%43 = OpLoad %mat2v2float %39
%48 = OpCompositeExtract %v2float %43 0
%49 = OpFMul %v2float %48 %45
%50 = OpCompositeExtract %v2float %43 1
%51 = OpFMul %v2float %50 %46
%52 = OpCompositeConstruct %mat2v2float %49 %51
OpStore %f22 %52
%56 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%59 = OpLoad %mat3v3float %56
%63 = OpCompositeExtract %v3float %59 0
%64 = OpFMul %v3float %63 %61
%65 = OpCompositeExtract %v3float %59 1
%66 = OpFMul %v3float %65 %61
%67 = OpCompositeExtract %v3float %59 2
%68 = OpFMul %v3float %67 %61
%69 = OpCompositeConstruct %mat3v3float %64 %66 %68
OpStore %h33 %69
%72 = OpFOrdEqual %v2bool %34 %34
%73 = OpAll %bool %72
%74 = OpFOrdEqual %v2bool %35 %35
%75 = OpAll %bool %74
%76 = OpLogicalAnd %bool %73 %75
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpLoad %mat2v2float %f22
%83 = OpCompositeExtract %v2float %79 0
%84 = OpFOrdEqual %v2bool %83 %45
%85 = OpAll %bool %84
%86 = OpCompositeExtract %v2float %79 1
%87 = OpFOrdEqual %v2bool %86 %81
%88 = OpAll %bool %87
%89 = OpLogicalAnd %bool %85 %88
OpBranch %78
%78 = OpLabel
%90 = OpPhi %bool %false %28 %89 %77
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %mat3v3float %h33
%105 = OpCompositeExtract %v3float %93 0
%106 = OpFOrdEqual %v3bool %105 %100
%107 = OpAll %bool %106
%108 = OpCompositeExtract %v3float %93 1
%109 = OpFOrdEqual %v3bool %108 %101
%110 = OpAll %bool %109
%111 = OpLogicalAnd %bool %107 %110
%112 = OpCompositeExtract %v3float %93 2
%113 = OpFOrdEqual %v3bool %112 %102
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %111 %114
OpBranch %92
%92 = OpLabel
%116 = OpPhi %bool %false %78 %115 %91
OpSelectionMerge %121 None
OpBranchConditional %116 %119 %120
%119 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%125 = OpLoad %v4float %122
OpStore %117 %125
OpBranch %121
%120 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%128 = OpLoad %v4float %126
OpStore %117 %128
OpBranch %121
%121 = OpLabel
%129 = OpLoad %v4float %117
OpReturnValue %129
OpFunctionEnd
