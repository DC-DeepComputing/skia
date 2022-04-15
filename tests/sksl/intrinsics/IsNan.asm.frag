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
OpName %valueIsNaN "valueIsNaN"
OpName %valueIsNumber "valueIsNumber"
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
OpDecorate %valueIsNaN RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %valueIsNumber RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%34 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_1 = OpConstant %float 1
%40 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
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
%valueIsNaN = OpVariable %_ptr_Function_v4float Function
%valueIsNumber = OpVariable %_ptr_Function_v4float Function
%99 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
%33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
%35 = OpFDiv %v4float %34 %33
OpStore %valueIsNaN %35
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %38
%41 = OpFDiv %v4float %40 %39
OpStore %valueIsNumber %41
%44 = OpCompositeExtract %float %35 0
%43 = OpIsNan %bool %44
OpSelectionMerge %46 None
OpBranchConditional %43 %45 %46
%45 = OpLabel
%49 = OpLoad %v4float %valueIsNaN
%50 = OpVectorShuffle %v2float %49 %49 0 1
%48 = OpIsNan %v2bool %50
%47 = OpAll %bool %48
OpBranch %46
%46 = OpLabel
%52 = OpPhi %bool %false %25 %47 %45
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
%57 = OpLoad %v4float %valueIsNaN
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%56 = OpIsNan %v3bool %58
%55 = OpAll %bool %56
OpBranch %54
%54 = OpLabel
%61 = OpPhi %bool %false %46 %55 %53
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%66 = OpLoad %v4float %valueIsNaN
%65 = OpIsNan %v4bool %66
%64 = OpAll %bool %65
OpBranch %63
%63 = OpLabel
%68 = OpPhi %bool %false %54 %64 %62
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%73 = OpLoad %v4float %valueIsNumber
%74 = OpCompositeExtract %float %73 0
%72 = OpIsNan %bool %74
%71 = OpLogicalNot %bool %72
OpBranch %70
%70 = OpLabel
%75 = OpPhi %bool %false %63 %71 %69
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%81 = OpLoad %v4float %valueIsNumber
%82 = OpVectorShuffle %v2float %81 %81 0 1
%80 = OpIsNan %v2bool %82
%79 = OpAny %bool %80
%78 = OpLogicalNot %bool %79
OpBranch %77
%77 = OpLabel
%83 = OpPhi %bool %false %70 %78 %76
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%89 = OpLoad %v4float %valueIsNumber
%90 = OpVectorShuffle %v3float %89 %89 0 1 2
%88 = OpIsNan %v3bool %90
%87 = OpAny %bool %88
%86 = OpLogicalNot %bool %87
OpBranch %85
%85 = OpLabel
%91 = OpPhi %bool %false %77 %86 %84
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%97 = OpLoad %v4float %valueIsNumber
%96 = OpIsNan %v4bool %97
%95 = OpAny %bool %96
%94 = OpLogicalNot %bool %95
OpBranch %93
%93 = OpLabel
%98 = OpPhi %bool %false %85 %94 %92
OpSelectionMerge %102 None
OpBranchConditional %98 %100 %101
%100 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %103
OpStore %99 %105
OpBranch %102
%101 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%108 = OpLoad %v4float %106
OpStore %99 %108
OpBranch %102
%102 = OpLabel
%109 = OpLoad %v4float %99
OpReturnValue %109
OpFunctionEnd
