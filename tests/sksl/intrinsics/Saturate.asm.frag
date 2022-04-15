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
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
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
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v4float %float_0 %float_0 %float_0_75 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%46 = OpConstantComposite %v2float %float_1 %float_1
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%60 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%61 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%v3bool = OpTypeVector %bool 3
%73 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%74 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%95 = OpConstantComposite %v3float %float_0 %float_0 %float_0_75
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
%107 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %30
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%32 = OpExtInst %float %1 FClamp %38 %float_0 %float_1
%39 = OpFOrdEqual %bool %32 %float_0
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpVectorShuffle %v2float %44 %44 0 1
%42 = OpExtInst %v2float %1 FClamp %45 %19 %46
%47 = OpLoad %v4float %expected
%48 = OpVectorShuffle %v2float %47 %47 0 1
%49 = OpFOrdEqual %v2bool %42 %48
%51 = OpAll %bool %49
OpBranch %41
%41 = OpLabel
%52 = OpPhi %bool %false %25 %51 %40
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%57 = OpLoad %v4float %56
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%55 = OpExtInst %v3float %1 FClamp %58 %60 %61
%62 = OpLoad %v4float %expected
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%64 = OpFOrdEqual %v3bool %55 %63
%66 = OpAll %bool %64
OpBranch %54
%54 = OpLabel
%67 = OpPhi %bool %false %41 %66 %53
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%70 = OpExtInst %v4float %1 FClamp %72 %73 %74
%75 = OpLoad %v4float %expected
%76 = OpFOrdEqual %v4bool %70 %75
%78 = OpAll %bool %76
OpBranch %69
%69 = OpLabel
%79 = OpPhi %bool %false %54 %78 %68
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %v4float %expected
%83 = OpCompositeExtract %float %82 0
%84 = OpFOrdEqual %bool %float_0 %83
OpBranch %81
%81 = OpLabel
%85 = OpPhi %bool %false %69 %84 %80
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpLoad %v4float %expected
%89 = OpVectorShuffle %v2float %88 %88 0 1
%90 = OpFOrdEqual %v2bool %19 %89
%91 = OpAll %bool %90
OpBranch %87
%87 = OpLabel
%92 = OpPhi %bool %false %81 %91 %86
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v4float %expected
%97 = OpVectorShuffle %v3float %96 %96 0 1 2
%98 = OpFOrdEqual %v3bool %95 %97
%99 = OpAll %bool %98
OpBranch %94
%94 = OpLabel
%100 = OpPhi %bool %false %87 %99 %93
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpLoad %v4float %expected
%104 = OpFOrdEqual %v4bool %30 %103
%105 = OpAll %bool %104
OpBranch %102
%102 = OpLabel
%106 = OpPhi %bool %false %94 %105 %101
OpSelectionMerge %110 None
OpBranchConditional %106 %108 %109
%108 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%113 = OpLoad %v4float %111
OpStore %107 %113
OpBranch %110
%109 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%116 = OpLoad %v4float %114
OpStore %107 %116
OpBranch %110
%110 = OpLabel
%117 = OpLoad %v4float %107
OpReturnValue %117
OpFunctionEnd
