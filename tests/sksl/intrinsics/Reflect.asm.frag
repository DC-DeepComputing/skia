OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "I"
OpMemberName %_UniformBuffer 1 "N"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedX "expectedX"
OpName %expectedXY "expectedXY"
OpName %expectedXYZ "expectedXYZ"
OpName %expectedXYZW "expectedXYZW"
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
OpDecorate %expectedX RelaxedPrecision
OpDecorate %expectedXY RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %expectedXYZ RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %expectedXYZW RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_n49 = OpConstant %float -49
%float_n169 = OpConstant %float -169
%float_202 = OpConstant %float 202
%34 = OpConstantComposite %v2float %float_n169 %float_202
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_n379 = OpConstant %float -379
%float_454 = OpConstant %float 454
%float_n529 = OpConstant %float -529
%41 = OpConstantComposite %v3float %float_n379 %float_454 %float_n529
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n699 = OpConstant %float -699
%float_838 = OpConstant %float 838
%float_n977 = OpConstant %float -977
%float_1116 = OpConstant %float 1116
%48 = OpConstantComposite %v4float %float_n699 %float_838 %float_n977 %float_1116
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
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
%expectedX = OpVariable %_ptr_Function_float Function
%expectedXY = OpVariable %_ptr_Function_v2float Function
%expectedXYZ = OpVariable %_ptr_Function_v3float Function
%expectedXYZW = OpVariable %_ptr_Function_v4float Function
%126 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedX %float_n49
OpStore %expectedXY %34
OpStore %expectedXYZ %41
OpStore %expectedXYZW %48
%51 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%55 = OpLoad %v4float %51
%56 = OpCompositeExtract %float %55 0
%57 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%59 = OpLoad %v4float %57
%60 = OpCompositeExtract %float %59 0
%50 = OpExtInst %float %1 Reflect %56 %60
%61 = OpLoad %float %expectedX
%62 = OpFOrdEqual %bool %50 %61
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v2float %67 %67 0 1
%69 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 0 1
%65 = OpExtInst %v2float %1 Reflect %68 %71
%72 = OpLoad %v2float %expectedXY
%73 = OpFOrdEqual %v2bool %65 %72
%75 = OpAll %bool %73
OpBranch %64
%64 = OpLabel
%76 = OpPhi %bool %false %27 %75 %63
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%80 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%81 = OpLoad %v4float %80
%82 = OpVectorShuffle %v3float %81 %81 0 1 2
%83 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%84 = OpLoad %v4float %83
%85 = OpVectorShuffle %v3float %84 %84 0 1 2
%79 = OpExtInst %v3float %1 Reflect %82 %85
%86 = OpLoad %v3float %expectedXYZ
%87 = OpFOrdEqual %v3bool %79 %86
%89 = OpAll %bool %87
OpBranch %78
%78 = OpLabel
%90 = OpPhi %bool %false %64 %89 %77
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%95 = OpLoad %v4float %94
%96 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%97 = OpLoad %v4float %96
%93 = OpExtInst %v4float %1 Reflect %95 %97
%98 = OpLoad %v4float %expectedXYZW
%99 = OpFOrdEqual %v4bool %93 %98
%101 = OpAll %bool %99
OpBranch %92
%92 = OpLabel
%102 = OpPhi %bool %false %78 %101 %91
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%105 = OpLoad %float %expectedX
%106 = OpFOrdEqual %bool %float_n49 %105
OpBranch %104
%104 = OpLabel
%107 = OpPhi %bool %false %92 %106 %103
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpLoad %v2float %expectedXY
%111 = OpFOrdEqual %v2bool %34 %110
%112 = OpAll %bool %111
OpBranch %109
%109 = OpLabel
%113 = OpPhi %bool %false %104 %112 %108
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%116 = OpLoad %v3float %expectedXYZ
%117 = OpFOrdEqual %v3bool %41 %116
%118 = OpAll %bool %117
OpBranch %115
%115 = OpLabel
%119 = OpPhi %bool %false %109 %118 %114
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%122 = OpLoad %v4float %expectedXYZW
%123 = OpFOrdEqual %v4bool %48 %122
%124 = OpAll %bool %123
OpBranch %121
%121 = OpLabel
%125 = OpPhi %bool %false %115 %124 %120
OpSelectionMerge %129 None
OpBranchConditional %125 %127 %128
%127 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%132 = OpLoad %v4float %130
OpStore %126 %132
OpBranch %129
%128 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %12 %int_3
%135 = OpLoad %v4float %133
OpStore %126 %135
OpBranch %129
%129 = OpLabel
%136 = OpLoad %v4float %126
OpReturnValue %136
OpFunctionEnd
