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
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%83 = OpConstantComposite %v2float %float_n1 %float_0
%91 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
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
%103 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %30
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%32 = OpExtInst %float %1 FSign %38
%39 = OpFOrdEqual %bool %32 %float_n1
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpVectorShuffle %v2float %44 %44 0 1
%42 = OpExtInst %v2float %1 FSign %45
%46 = OpLoad %v4float %expected
%47 = OpVectorShuffle %v2float %46 %46 0 1
%48 = OpFOrdEqual %v2bool %42 %47
%50 = OpAll %bool %48
OpBranch %41
%41 = OpLabel
%51 = OpPhi %bool %false %25 %50 %40
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%56 = OpLoad %v4float %55
%57 = OpVectorShuffle %v3float %56 %56 0 1 2
%54 = OpExtInst %v3float %1 FSign %57
%59 = OpLoad %v4float %expected
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%61 = OpFOrdEqual %v3bool %54 %60
%63 = OpAll %bool %61
OpBranch %53
%53 = OpLabel
%64 = OpPhi %bool %false %41 %63 %52
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%69 = OpLoad %v4float %68
%67 = OpExtInst %v4float %1 FSign %69
%70 = OpLoad %v4float %expected
%71 = OpFOrdEqual %v4bool %67 %70
%73 = OpAll %bool %71
OpBranch %66
%66 = OpLabel
%74 = OpPhi %bool %false %53 %73 %65
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %v4float %expected
%78 = OpCompositeExtract %float %77 0
%79 = OpFOrdEqual %bool %float_n1 %78
OpBranch %76
%76 = OpLabel
%80 = OpPhi %bool %false %66 %79 %75
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpLoad %v4float %expected
%85 = OpVectorShuffle %v2float %84 %84 0 1
%86 = OpFOrdEqual %v2bool %83 %85
%87 = OpAll %bool %86
OpBranch %82
%82 = OpLabel
%88 = OpPhi %bool %false %76 %87 %81
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpLoad %v4float %expected
%93 = OpVectorShuffle %v3float %92 %92 0 1 2
%94 = OpFOrdEqual %v3bool %91 %93
%95 = OpAll %bool %94
OpBranch %90
%90 = OpLabel
%96 = OpPhi %bool %false %82 %95 %89
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%99 = OpLoad %v4float %expected
%100 = OpFOrdEqual %v4bool %30 %99
%101 = OpAll %bool %100
OpBranch %98
%98 = OpLabel
%102 = OpPhi %bool %false %90 %101 %97
OpSelectionMerge %106 None
OpBranchConditional %102 %104 %105
%104 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%109 = OpLoad %v4float %107
OpStore %103 %109
OpBranch %106
%105 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%112 = OpLoad %v4float %110
OpStore %103 %112
OpBranch %106
%106 = OpLabel
%113 = OpLoad %v4float %103
OpReturnValue %113
OpFunctionEnd
