OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
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
OpDecorate %34 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
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
%float_5 = OpConstant %float 5
%float_17 = OpConstant %float 17
%float_38 = OpConstant %float 38
%float_70 = OpConstant %float 70
%32 = OpConstantComposite %v4float %float_5 %float_17 %float_38 %float_70
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
%108 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %32
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 0
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %41
%44 = OpCompositeExtract %float %43 0
%34 = OpFMul %float %40 %44
%45 = OpFOrdEqual %bool %34 %float_5
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 0 1
%48 = OpDot %float %51 %54
%55 = OpLoad %v4float %expected
%56 = OpCompositeExtract %float %55 1
%57 = OpFOrdEqual %bool %48 %56
OpBranch %47
%47 = OpLabel
%58 = OpPhi %bool %false %25 %57 %46
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%61 = OpDot %float %64 %68
%69 = OpLoad %v4float %expected
%70 = OpCompositeExtract %float %69 2
%71 = OpFOrdEqual %bool %61 %70
OpBranch %60
%60 = OpLabel
%72 = OpPhi %bool %false %47 %71 %59
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%79 = OpLoad %v4float %78
%75 = OpDot %float %77 %79
%80 = OpLoad %v4float %expected
%81 = OpCompositeExtract %float %80 3
%82 = OpFOrdEqual %bool %75 %81
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %60 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpLoad %v4float %expected
%87 = OpCompositeExtract %float %86 0
%88 = OpFOrdEqual %bool %float_5 %87
OpBranch %85
%85 = OpLabel
%89 = OpPhi %bool %false %74 %88 %84
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpLoad %v4float %expected
%93 = OpCompositeExtract %float %92 1
%94 = OpFOrdEqual %bool %float_17 %93
OpBranch %91
%91 = OpLabel
%95 = OpPhi %bool %false %85 %94 %90
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpLoad %v4float %expected
%99 = OpCompositeExtract %float %98 2
%100 = OpFOrdEqual %bool %float_38 %99
OpBranch %97
%97 = OpLabel
%101 = OpPhi %bool %false %91 %100 %96
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%104 = OpLoad %v4float %expected
%105 = OpCompositeExtract %float %104 3
%106 = OpFOrdEqual %bool %float_70 %105
OpBranch %103
%103 = OpLabel
%107 = OpPhi %bool %false %97 %106 %102
OpSelectionMerge %111 None
OpBranchConditional %107 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%114 = OpLoad %v4float %112
OpStore %108 %114
OpBranch %111
%110 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%117 = OpLoad %v4float %115
OpStore %108 %117
OpBranch %111
%111 = OpLabel
%118 = OpLoad %v4float %108
OpReturnValue %118
OpFunctionEnd
