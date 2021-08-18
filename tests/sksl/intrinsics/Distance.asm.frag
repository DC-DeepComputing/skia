OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "pos1"
OpMemberName %_UniformBuffer 1 "pos2"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
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
%float_3 = OpConstant %float 3
%float_5 = OpConstant %float 5
%float_13 = OpConstant %float 13
%33 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
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
%111 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %33
%36 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%40 = OpLoad %v4float %36
%41 = OpCompositeExtract %float %40 0
%42 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%44 = OpLoad %v4float %42
%45 = OpCompositeExtract %float %44 0
%35 = OpExtInst %float %1 Distance %41 %45
%46 = OpLoad %v4float %expected
%47 = OpCompositeExtract %float %46 0
%48 = OpFOrdEqual %bool %35 %47
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 0 1
%55 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%56 = OpLoad %v4float %55
%57 = OpVectorShuffle %v2float %56 %56 0 1
%51 = OpExtInst %float %1 Distance %54 %57
%58 = OpLoad %v4float %expected
%59 = OpCompositeExtract %float %58 1
%60 = OpFOrdEqual %bool %51 %59
OpBranch %50
%50 = OpLabel
%61 = OpPhi %bool %false %27 %60 %49
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%69 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v3float %70 %70 0 1 2
%64 = OpExtInst %float %1 Distance %67 %71
%72 = OpLoad %v4float %expected
%73 = OpCompositeExtract %float %72 2
%74 = OpFOrdEqual %bool %64 %73
OpBranch %63
%63 = OpLabel
%75 = OpPhi %bool %false %50 %74 %62
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%80 = OpLoad %v4float %79
%81 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%82 = OpLoad %v4float %81
%78 = OpExtInst %float %1 Distance %80 %82
%83 = OpLoad %v4float %expected
%84 = OpCompositeExtract %float %83 3
%85 = OpFOrdEqual %bool %78 %84
OpBranch %77
%77 = OpLabel
%86 = OpPhi %bool %false %63 %85 %76
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %v4float %expected
%90 = OpCompositeExtract %float %89 0
%91 = OpFOrdEqual %bool %float_3 %90
OpBranch %88
%88 = OpLabel
%92 = OpPhi %bool %false %77 %91 %87
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpLoad %v4float %expected
%96 = OpCompositeExtract %float %95 1
%97 = OpFOrdEqual %bool %float_3 %96
OpBranch %94
%94 = OpLabel
%98 = OpPhi %bool %false %88 %97 %93
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %v4float %expected
%102 = OpCompositeExtract %float %101 2
%103 = OpFOrdEqual %bool %float_5 %102
OpBranch %100
%100 = OpLabel
%104 = OpPhi %bool %false %94 %103 %99
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpLoad %v4float %expected
%108 = OpCompositeExtract %float %107 3
%109 = OpFOrdEqual %bool %float_13 %108
OpBranch %106
%106 = OpLabel
%110 = OpPhi %bool %false %100 %109 %105
OpSelectionMerge %114 None
OpBranchConditional %110 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%117 = OpLoad %v4float %115
OpStore %111 %117
OpBranch %114
%113 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%120 = OpLoad %v4float %118
OpStore %111 %120
OpBranch %114
%114 = OpLabel
%121 = OpLoad %v4float %111
OpReturnValue %121
OpFunctionEnd
