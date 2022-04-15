OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputVal"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedVec "expectedVec"
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
OpDecorate %expectedVec RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%82 = OpConstantComposite %v2float %float_0 %float_1
%90 = OpConstantComposite %v3float %float_0 %float_1 %float_0
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
%expectedVec = OpVariable %_ptr_Function_v4float Function
%102 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedVec %29
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpExtInst %float %1 Normalize %37
%38 = OpFOrdEqual %bool %31 %float_1
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%43 = OpLoad %v4float %42
%44 = OpVectorShuffle %v2float %43 %43 0 1
%41 = OpExtInst %v2float %1 Normalize %44
%45 = OpLoad %v4float %expectedVec
%46 = OpVectorShuffle %v2float %45 %45 0 1
%47 = OpFOrdEqual %v2bool %41 %46
%49 = OpAll %bool %47
OpBranch %40
%40 = OpLabel
%50 = OpPhi %bool %false %25 %49 %39
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%55 = OpLoad %v4float %54
%56 = OpVectorShuffle %v3float %55 %55 0 1 2
%53 = OpExtInst %v3float %1 Normalize %56
%58 = OpLoad %v4float %expectedVec
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%60 = OpFOrdEqual %v3bool %53 %59
%62 = OpAll %bool %60
OpBranch %52
%52 = OpLabel
%63 = OpPhi %bool %false %40 %62 %51
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%68 = OpLoad %v4float %67
%66 = OpExtInst %v4float %1 Normalize %68
%69 = OpLoad %v4float %expectedVec
%70 = OpFOrdEqual %v4bool %66 %69
%72 = OpAll %bool %70
OpBranch %65
%65 = OpLabel
%73 = OpPhi %bool %false %52 %72 %64
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %v4float %expectedVec
%77 = OpCompositeExtract %float %76 0
%78 = OpFOrdEqual %bool %float_1 %77
OpBranch %75
%75 = OpLabel
%79 = OpPhi %bool %false %65 %78 %74
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpLoad %v4float %expectedVec
%84 = OpVectorShuffle %v2float %83 %83 1 0
%85 = OpFOrdEqual %v2bool %82 %84
%86 = OpAll %bool %85
OpBranch %81
%81 = OpLabel
%87 = OpPhi %bool %false %75 %86 %80
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%91 = OpLoad %v4float %expectedVec
%92 = OpVectorShuffle %v3float %91 %91 2 0 1
%93 = OpFOrdEqual %v3bool %90 %92
%94 = OpAll %bool %93
OpBranch %89
%89 = OpLabel
%95 = OpPhi %bool %false %81 %94 %88
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpLoad %v4float %expectedVec
%99 = OpFOrdEqual %v4bool %29 %98
%100 = OpAll %bool %99
OpBranch %97
%97 = OpLabel
%101 = OpPhi %bool %false %89 %100 %96
OpSelectionMerge %105 None
OpBranchConditional %101 %103 %104
%103 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%108 = OpLoad %v4float %106
OpStore %102 %108
OpBranch %105
%104 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%111 = OpLoad %v4float %109
OpStore %102 %111
OpBranch %105
%105 = OpLabel
%112 = OpLoad %v4float %102
OpReturnValue %112
OpFunctionEnd
