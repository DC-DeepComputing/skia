OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInput"
OpMemberName %_UniformBuffer 1 "testMatrix2x2"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inputVal "inputVal"
OpName %expectedB "expectedB"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 48
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %float %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_n1 = OpConstant %float -1
%41 = OpConstantComposite %v4float %float_1 %float_1 %float_n1 %float_n1
%uint = OpTypeInt 32 0
%v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%uint_1065353216 = OpConstant %uint 1065353216
%uint_1073741824 = OpConstant %uint 1073741824
%uint_3225419776 = OpConstant %uint 3225419776
%uint_3229614080 = OpConstant %uint 3229614080
%51 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
%false = OpConstantFalse %bool
%v2uint = OpTypeVector %uint 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3uint = OpTypeVector %uint 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%inputVal = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4uint Function
%90 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_1
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
%42 = OpFMul %v4float %38 %41
OpStore %inputVal %42
OpStore %expectedB %51
%53 = OpCompositeExtract %float %42 0
%54 = OpBitcast %float %uint_1065353216
%55 = OpFOrdEqual %bool %53 %54
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%58 = OpLoad %v4float %inputVal
%59 = OpVectorShuffle %v2float %58 %58 0 1
%61 = OpLoad %v4uint %expectedB
%62 = OpVectorShuffle %v2uint %61 %61 0 1
%60 = OpBitcast %v2float %62
%64 = OpFOrdEqual %v2bool %59 %60
%66 = OpAll %bool %64
OpBranch %57
%57 = OpLabel
%67 = OpPhi %bool %false %26 %66 %56
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpLoad %v4float %inputVal
%71 = OpVectorShuffle %v3float %70 %70 0 1 2
%74 = OpLoad %v4uint %expectedB
%75 = OpVectorShuffle %v3uint %74 %74 0 1 2
%73 = OpBitcast %v3float %75
%77 = OpFOrdEqual %v3bool %71 %73
%79 = OpAll %bool %77
OpBranch %69
%69 = OpLabel
%80 = OpPhi %bool %false %57 %79 %68
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %v4float %inputVal
%85 = OpLoad %v4uint %expectedB
%84 = OpBitcast %v4float %85
%86 = OpFOrdEqual %v4bool %83 %84
%88 = OpAll %bool %86
OpBranch %82
%82 = OpLabel
%89 = OpPhi %bool %false %69 %88 %81
OpSelectionMerge %93 None
OpBranchConditional %89 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%97 = OpLoad %v4float %94
OpStore %90 %97
OpBranch %93
%92 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%100 = OpLoad %v4float %98
OpStore %90 %100
OpBranch %93
%93 = OpLabel
%101 = OpLoad %v4float %90
OpReturnValue %101
OpFunctionEnd
