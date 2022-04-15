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
OpName %_0_ok "_0_ok"
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
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%45 = OpConstantComposite %v2float %float_1 %float_2
%46 = OpConstantComposite %v2float %float_3 %float_4
%47 = OpConstantComposite %mat2v2float %45 %46
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%68 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%69 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%70 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%71 = OpConstantComposite %mat3v3float %68 %69 %70
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%90 = OpConstantComposite %v2float %float_100 %float_0
%91 = OpConstantComposite %v2float %float_0 %float_100
%92 = OpConstantComposite %mat2v2float %90 %91
%105 = OpConstantComposite %v3float %float_9 %float_8 %float_7
%106 = OpConstantComposite %v3float %float_6 %float_5 %float_4
%107 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%108 = OpConstantComposite %mat3v3float %105 %106 %107
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
%_0_ok = OpVariable %_ptr_Function_bool Function
%121 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%33 = OpLoad %bool %_0_ok
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%40 = OpLoad %mat2v2float %36
%49 = OpCompositeExtract %v2float %40 0
%50 = OpFOrdEqual %v2bool %49 %45
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v2float %40 1
%53 = OpFOrdEqual %v2bool %52 %46
%54 = OpAll %bool %53
%55 = OpLogicalAnd %bool %51 %54
OpBranch %35
%35 = OpLabel
%56 = OpPhi %bool %false %28 %55 %34
OpStore %_0_ok %56
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%62 = OpLoad %mat3v3float %59
%73 = OpCompositeExtract %v3float %62 0
%74 = OpFOrdEqual %v3bool %73 %68
%75 = OpAll %bool %74
%76 = OpCompositeExtract %v3float %62 1
%77 = OpFOrdEqual %v3bool %76 %69
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %75 %78
%80 = OpCompositeExtract %v3float %62 2
%81 = OpFOrdEqual %v3bool %80 %70
%82 = OpAll %bool %81
%83 = OpLogicalAnd %bool %79 %82
OpBranch %58
%58 = OpLabel
%84 = OpPhi %bool %false %35 %83 %57
OpStore %_0_ok %84
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%88 = OpLoad %mat2v2float %87
%93 = OpCompositeExtract %v2float %88 0
%94 = OpFUnordNotEqual %v2bool %93 %90
%95 = OpAny %bool %94
%96 = OpCompositeExtract %v2float %88 1
%97 = OpFUnordNotEqual %v2bool %96 %91
%98 = OpAny %bool %97
%99 = OpLogicalOr %bool %95 %98
OpBranch %86
%86 = OpLabel
%100 = OpPhi %bool %false %58 %99 %85
OpStore %_0_ok %100
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%104 = OpLoad %mat3v3float %103
%109 = OpCompositeExtract %v3float %104 0
%110 = OpFUnordNotEqual %v3bool %109 %105
%111 = OpAny %bool %110
%112 = OpCompositeExtract %v3float %104 1
%113 = OpFUnordNotEqual %v3bool %112 %106
%114 = OpAny %bool %113
%115 = OpLogicalOr %bool %111 %114
%116 = OpCompositeExtract %v3float %104 2
%117 = OpFUnordNotEqual %v3bool %116 %107
%118 = OpAny %bool %117
%119 = OpLogicalOr %bool %115 %118
OpBranch %102
%102 = OpLabel
%120 = OpPhi %bool %false %86 %119 %101
OpStore %_0_ok %120
OpSelectionMerge %125 None
OpBranchConditional %120 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%129 = OpLoad %v4float %126
OpStore %121 %129
OpBranch %125
%124 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %130
OpStore %121 %132
OpBranch %125
%125 = OpLabel
%133 = OpLoad %v4float %121
OpReturnValue %133
OpFunctionEnd
