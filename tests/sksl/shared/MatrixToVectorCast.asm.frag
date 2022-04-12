OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %31 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%48 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%v4bool = OpTypeVector %bool 4
%63 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%v4int = OpTypeVector %int 4
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%90 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
%113 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
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
%ok = OpVariable %_ptr_Function_bool Function
%118 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%31 = OpLoad %bool %ok
OpSelectionMerge %33 None
OpBranchConditional %31 %32 %33
%32 = OpLabel
%34 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%38 = OpLoad %mat2v2float %34
%39 = OpCompositeExtract %float %38 0 0
%40 = OpCompositeExtract %float %38 0 1
%41 = OpCompositeExtract %float %38 1 0
%42 = OpCompositeExtract %float %38 1 1
%43 = OpCompositeConstruct %v4float %39 %40 %41 %42
%49 = OpFOrdEqual %v4bool %43 %48
%51 = OpAll %bool %49
OpBranch %33
%33 = OpLabel
%52 = OpPhi %bool %false %26 %51 %32
OpStore %ok %52
%53 = OpLoad %bool %ok
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%57 = OpLoad %mat2v2float %56
%58 = OpCompositeExtract %float %57 0 0
%59 = OpCompositeExtract %float %57 0 1
%60 = OpCompositeExtract %float %57 1 0
%61 = OpCompositeExtract %float %57 1 1
%62 = OpCompositeConstruct %v4float %58 %59 %60 %61
%64 = OpFOrdEqual %v4bool %62 %63
%65 = OpAll %bool %64
OpBranch %55
%55 = OpLabel
%66 = OpPhi %bool %false %33 %65 %54
OpStore %ok %66
%67 = OpLoad %bool %ok
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%70 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%71 = OpLoad %mat2v2float %70
%72 = OpCompositeExtract %float %71 0 0
%73 = OpCompositeExtract %float %71 0 1
%74 = OpCompositeExtract %float %71 1 0
%75 = OpCompositeExtract %float %71 1 1
%76 = OpCompositeConstruct %v4float %72 %73 %74 %75
%77 = OpCompositeExtract %float %76 0
%78 = OpConvertFToS %int %77
%79 = OpCompositeExtract %float %76 1
%80 = OpConvertFToS %int %79
%81 = OpCompositeExtract %float %76 2
%82 = OpConvertFToS %int %81
%83 = OpCompositeExtract %float %76 3
%84 = OpConvertFToS %int %83
%86 = OpCompositeConstruct %v4int %78 %80 %82 %84
%91 = OpIEqual %v4bool %86 %90
%92 = OpAll %bool %91
OpBranch %69
%69 = OpLabel
%93 = OpPhi %bool %false %55 %92 %68
OpStore %ok %93
%94 = OpLoad %bool %ok
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%98 = OpLoad %mat2v2float %97
%99 = OpCompositeExtract %float %98 0 0
%100 = OpCompositeExtract %float %98 0 1
%101 = OpCompositeExtract %float %98 1 0
%102 = OpCompositeExtract %float %98 1 1
%103 = OpCompositeConstruct %v4float %99 %100 %101 %102
%104 = OpCompositeExtract %float %103 0
%105 = OpFUnordNotEqual %bool %104 %float_0
%106 = OpCompositeExtract %float %103 1
%107 = OpFUnordNotEqual %bool %106 %float_0
%108 = OpCompositeExtract %float %103 2
%109 = OpFUnordNotEqual %bool %108 %float_0
%110 = OpCompositeExtract %float %103 3
%111 = OpFUnordNotEqual %bool %110 %float_0
%112 = OpCompositeConstruct %v4bool %105 %107 %109 %111
%114 = OpLogicalEqual %v4bool %112 %113
%115 = OpAll %bool %114
OpBranch %96
%96 = OpLabel
%116 = OpPhi %bool %false %69 %115 %95
OpStore %ok %116
%117 = OpLoad %bool %ok
OpSelectionMerge %122 None
OpBranchConditional %117 %120 %121
%120 = OpLabel
%123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%126 = OpLoad %v4float %123
OpStore %118 %126
OpBranch %122
%121 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%128 = OpLoad %v4float %127
OpStore %118 %128
OpBranch %122
%122 = OpLabel
%129 = OpLoad %v4float %118
OpReturnValue %129
OpFunctionEnd
