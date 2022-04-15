OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
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
%float_3 = OpConstant %float 3
%float_5 = OpConstant %float 5
%float_13 = OpConstant %float 13
%31 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%107 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
%34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%38 = OpLoad %v4float %34
%39 = OpCompositeExtract %float %38 0
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %40
%43 = OpCompositeExtract %float %42 0
%33 = OpExtInst %float %1 Distance %39 %43
%44 = OpFOrdEqual %bool %33 %float_3
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
%47 = OpExtInst %float %1 Distance %50 %53
%54 = OpLoad %v4float %expected
%55 = OpCompositeExtract %float %54 1
%56 = OpFOrdEqual %bool %47 %55
OpBranch %46
%46 = OpLabel
%57 = OpPhi %bool %false %25 %56 %45
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%60 = OpExtInst %float %1 Distance %63 %67
%68 = OpLoad %v4float %expected
%69 = OpCompositeExtract %float %68 2
%70 = OpFOrdEqual %bool %60 %69
OpBranch %59
%59 = OpLabel
%71 = OpPhi %bool %false %46 %70 %58
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%76 = OpLoad %v4float %75
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%78 = OpLoad %v4float %77
%74 = OpExtInst %float %1 Distance %76 %78
%79 = OpLoad %v4float %expected
%80 = OpCompositeExtract %float %79 3
%81 = OpFOrdEqual %bool %74 %80
OpBranch %73
%73 = OpLabel
%82 = OpPhi %bool %false %59 %81 %72
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %v4float %expected
%86 = OpCompositeExtract %float %85 0
%87 = OpFOrdEqual %bool %float_3 %86
OpBranch %84
%84 = OpLabel
%88 = OpPhi %bool %false %73 %87 %83
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpLoad %v4float %expected
%92 = OpCompositeExtract %float %91 1
%93 = OpFOrdEqual %bool %float_3 %92
OpBranch %90
%90 = OpLabel
%94 = OpPhi %bool %false %84 %93 %89
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %v4float %expected
%98 = OpCompositeExtract %float %97 2
%99 = OpFOrdEqual %bool %float_5 %98
OpBranch %96
%96 = OpLabel
%100 = OpPhi %bool %false %90 %99 %95
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpLoad %v4float %expected
%104 = OpCompositeExtract %float %103 3
%105 = OpFOrdEqual %bool %float_13 %104
OpBranch %102
%102 = OpLabel
%106 = OpPhi %bool %false %96 %105 %101
OpSelectionMerge %110 None
OpBranchConditional %106 %108 %109
%108 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%113 = OpLoad %v4float %111
OpStore %107 %113
OpBranch %110
%109 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%116 = OpLoad %v4float %114
OpStore %107 %116
OpBranch %110
%110 = OpLabel
%117 = OpLoad %v4float %107
OpReturnValue %117
OpFunctionEnd
