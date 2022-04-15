OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %infiniteValue "infiniteValue"
OpName %finiteValue "finiteValue"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 48
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %infiniteValue RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %finiteValue RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float
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
%int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
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
%infiniteValue = OpVariable %_ptr_Function_v4float Function
%finiteValue = OpVariable %_ptr_Function_v4float Function
%117 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
%39 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %39
%43 = OpCompositeExtract %float %42 0
%45 = OpFDiv %float %float_1 %43
%46 = OpVectorTimesScalar %v4float %38 %45
OpStore %infiniteValue %46
%48 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%49 = OpLoad %mat2v2float %48
%50 = OpCompositeExtract %float %49 0 0
%51 = OpCompositeExtract %float %49 0 1
%52 = OpCompositeExtract %float %49 1 0
%53 = OpCompositeExtract %float %49 1 1
%54 = OpCompositeConstruct %v4float %50 %51 %52 %53
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 1
%58 = OpFDiv %float %float_1 %57
%59 = OpVectorTimesScalar %v4float %54 %58
OpStore %finiteValue %59
%62 = OpCompositeExtract %float %46 0
%61 = OpIsInf %bool %62
OpSelectionMerge %64 None
OpBranchConditional %61 %63 %64
%63 = OpLabel
%67 = OpLoad %v4float %infiniteValue
%68 = OpVectorShuffle %v2float %67 %67 0 1
%66 = OpIsInf %v2bool %68
%65 = OpAll %bool %66
OpBranch %64
%64 = OpLabel
%70 = OpPhi %bool %false %26 %65 %63
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%75 = OpLoad %v4float %infiniteValue
%76 = OpVectorShuffle %v3float %75 %75 0 1 2
%74 = OpIsInf %v3bool %76
%73 = OpAll %bool %74
OpBranch %72
%72 = OpLabel
%79 = OpPhi %bool %false %64 %73 %71
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%84 = OpLoad %v4float %infiniteValue
%83 = OpIsInf %v4bool %84
%82 = OpAll %bool %83
OpBranch %81
%81 = OpLabel
%86 = OpPhi %bool %false %72 %82 %80
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%91 = OpLoad %v4float %finiteValue
%92 = OpCompositeExtract %float %91 0
%90 = OpIsInf %bool %92
%89 = OpLogicalNot %bool %90
OpBranch %88
%88 = OpLabel
%93 = OpPhi %bool %false %81 %89 %87
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%99 = OpLoad %v4float %finiteValue
%100 = OpVectorShuffle %v2float %99 %99 0 1
%98 = OpIsInf %v2bool %100
%97 = OpAny %bool %98
%96 = OpLogicalNot %bool %97
OpBranch %95
%95 = OpLabel
%101 = OpPhi %bool %false %88 %96 %94
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%107 = OpLoad %v4float %finiteValue
%108 = OpVectorShuffle %v3float %107 %107 0 1 2
%106 = OpIsInf %v3bool %108
%105 = OpAny %bool %106
%104 = OpLogicalNot %bool %105
OpBranch %103
%103 = OpLabel
%109 = OpPhi %bool %false %95 %104 %102
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%115 = OpLoad %v4float %finiteValue
%114 = OpIsInf %v4bool %115
%113 = OpAny %bool %114
%112 = OpLogicalNot %bool %113
OpBranch %111
%111 = OpLabel
%116 = OpPhi %bool %false %103 %112 %110
OpSelectionMerge %120 None
OpBranchConditional %116 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%122 = OpLoad %v4float %121
OpStore %117 %122
OpBranch %120
%119 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%125 = OpLoad %v4float %123
OpStore %117 %125
OpBranch %120
%120 = OpLabel
%126 = OpLoad %v4float %117
OpReturnValue %126
OpFunctionEnd
