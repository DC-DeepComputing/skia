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
OpName %h44 "h44"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %h44 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
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
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_3 = OpConstant %float 3
%float_5_5 = OpConstant %float 5.5
%float_8 = OpConstant %float 8
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
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
%h44 = OpVariable %_ptr_Function_mat4v4float Function
%93 = OpVariable %_ptr_Function_v4float Function
%34 = OpCompositeConstruct %v2float %float_0 %float_5
%35 = OpCompositeConstruct %v2float %float_10 %float_15
%36 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %h22 %36
%44 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %float_0
%45 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%46 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5_5 %float_0
%47 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_8
%48 = OpCompositeConstruct %mat4v4float %44 %45 %46 %47
OpStore %h44 %48
%50 = OpLoad %mat2v2float %h22
%51 = OpCompositeConstruct %v2float %float_0 %float_5
%52 = OpCompositeConstruct %v2float %float_10 %float_15
%53 = OpCompositeConstruct %mat2v2float %51 %52
%55 = OpCompositeExtract %v2float %50 0
%56 = OpCompositeExtract %v2float %53 0
%57 = OpFOrdEqual %v2bool %55 %56
%58 = OpAll %bool %57
%59 = OpCompositeExtract %v2float %50 1
%60 = OpCompositeExtract %v2float %53 1
%61 = OpFOrdEqual %v2bool %59 %60
%62 = OpAll %bool %61
%63 = OpLogicalAnd %bool %58 %62
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%66 = OpLoad %mat4v4float %h44
%67 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %float_0
%68 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%69 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5_5 %float_0
%70 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_8
%71 = OpCompositeConstruct %mat4v4float %67 %68 %69 %70
%73 = OpCompositeExtract %v4float %66 0
%74 = OpCompositeExtract %v4float %71 0
%75 = OpFOrdEqual %v4bool %73 %74
%76 = OpAll %bool %75
%77 = OpCompositeExtract %v4float %66 1
%78 = OpCompositeExtract %v4float %71 1
%79 = OpFOrdEqual %v4bool %77 %78
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %76 %80
%82 = OpCompositeExtract %v4float %66 2
%83 = OpCompositeExtract %v4float %71 2
%84 = OpFOrdEqual %v4bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
%87 = OpCompositeExtract %v4float %66 3
%88 = OpCompositeExtract %v4float %71 3
%89 = OpFOrdEqual %v4bool %87 %88
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %86 %90
OpBranch %65
%65 = OpLabel
%92 = OpPhi %bool %false %28 %91 %64
OpSelectionMerge %97 None
OpBranchConditional %92 %95 %96
%95 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%102 = OpLoad %v4float %98
OpStore %93 %102
OpBranch %97
%96 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %103
OpStore %93 %105
OpBranch %97
%97 = OpLabel
%106 = OpLoad %v4float %93
OpReturnValue %106
OpFunctionEnd
