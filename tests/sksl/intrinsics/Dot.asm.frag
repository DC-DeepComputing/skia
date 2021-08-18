OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputA"
OpMemberName %_UniformBuffer 1 "inputB"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
OpDecorate %sk_FragCoord BuiltIn FragCoord
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
%float_5 = OpConstant %float 5
%float_17 = OpConstant %float 17
%float_38 = OpConstant %float 38
%float_70 = OpConstant %float 70
%34 = OpConstantComposite %v4float %float_5 %float_17 %float_38 %float_70
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
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
%expected = OpVariable %_ptr_Function_v4float Function
%112 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %34
%37 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%43 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%45 = OpLoad %v4float %43
%46 = OpCompositeExtract %float %45 0
%36 = OpFMul %float %42 %46
%47 = OpLoad %v4float %expected
%48 = OpCompositeExtract %float %47 0
%49 = OpFOrdEqual %bool %36 %48
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%54 = OpLoad %v4float %53
%55 = OpVectorShuffle %v2float %54 %54 0 1
%56 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%57 = OpLoad %v4float %56
%58 = OpVectorShuffle %v2float %57 %57 0 1
%52 = OpDot %float %55 %58
%59 = OpLoad %v4float %expected
%60 = OpCompositeExtract %float %59 1
%61 = OpFOrdEqual %bool %52 %60
OpBranch %51
%51 = OpLabel
%62 = OpPhi %bool %false %27 %61 %50
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%70 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%71 = OpLoad %v4float %70
%72 = OpVectorShuffle %v3float %71 %71 0 1 2
%65 = OpDot %float %68 %72
%73 = OpLoad %v4float %expected
%74 = OpCompositeExtract %float %73 2
%75 = OpFOrdEqual %bool %65 %74
OpBranch %64
%64 = OpLabel
%76 = OpPhi %bool %false %51 %75 %63
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%81 = OpLoad %v4float %80
%82 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%83 = OpLoad %v4float %82
%79 = OpDot %float %81 %83
%84 = OpLoad %v4float %expected
%85 = OpCompositeExtract %float %84 3
%86 = OpFOrdEqual %bool %79 %85
OpBranch %78
%78 = OpLabel
%87 = OpPhi %bool %false %64 %86 %77
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%90 = OpLoad %v4float %expected
%91 = OpCompositeExtract %float %90 0
%92 = OpFOrdEqual %bool %float_5 %91
OpBranch %89
%89 = OpLabel
%93 = OpPhi %bool %false %78 %92 %88
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%96 = OpLoad %v4float %expected
%97 = OpCompositeExtract %float %96 1
%98 = OpFOrdEqual %bool %float_17 %97
OpBranch %95
%95 = OpLabel
%99 = OpPhi %bool %false %89 %98 %94
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpLoad %v4float %expected
%103 = OpCompositeExtract %float %102 2
%104 = OpFOrdEqual %bool %float_38 %103
OpBranch %101
%101 = OpLabel
%105 = OpPhi %bool %false %95 %104 %100
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpLoad %v4float %expected
%109 = OpCompositeExtract %float %108 3
%110 = OpFOrdEqual %bool %float_70 %109
OpBranch %107
%107 = OpLabel
%111 = OpPhi %bool %false %101 %110 %106
OpSelectionMerge %115 None
OpBranchConditional %111 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%118 = OpLoad %v4float %116
OpStore %112 %118
OpBranch %115
%114 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%121 = OpLoad %v4float %119
OpStore %112 %121
OpBranch %115
%115 = OpLabel
%122 = OpLoad %v4float %112
OpReturnValue %122
OpFunctionEnd
