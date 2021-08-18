### Compilation failed:

error: SPIR-V validation error: Expected P type and Result Type to be the same: DPdx
  %135 = OpDPdx %v2float %152

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpMemberName %_UniformBuffer 3 "u_skRTFlip"
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
OpMemberDecorate %_UniformBuffer 3 Offset 16384
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%31 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%float_1 = OpConstant %float 1
%103 = OpConstantComposite %v2float %float_1 %float_1
%153 = OpConstantComposite %v2float %float_1 %float_0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%157 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
%34 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%38 = OpLoad %v4float %34
%39 = OpCompositeExtract %float %38 0
%33 = OpDPdx %float %39
%40 = OpLoad %v4float %expected
%41 = OpCompositeExtract %float %40 0
%42 = OpFOrdEqual %bool %33 %41
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v2float %47 %47 0 1
%45 = OpDPdx %v2float %48
%49 = OpLoad %v4float %expected
%50 = OpVectorShuffle %v2float %49 %49 0 1
%51 = OpFOrdEqual %v2bool %45 %50
%53 = OpAll %bool %51
OpBranch %44
%44 = OpLabel
%54 = OpPhi %bool %false %28 %53 %43
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%59 = OpLoad %v4float %58
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%57 = OpDPdx %v3float %60
%62 = OpLoad %v4float %expected
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%64 = OpFOrdEqual %v3bool %57 %63
%66 = OpAll %bool %64
OpBranch %56
%56 = OpLabel
%67 = OpPhi %bool %false %44 %66 %55
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%72 = OpLoad %v4float %71
%70 = OpDPdx %v4float %72
%73 = OpLoad %v4float %expected
%74 = OpFOrdEqual %v4bool %70 %73
%76 = OpAll %bool %74
OpBranch %69
%69 = OpLabel
%77 = OpPhi %bool %false %56 %76 %68
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%82 = OpLoad %v4float %sk_FragCoord
%83 = OpCompositeExtract %float %82 0
%85 = OpAccessChain %_ptr_Uniform_v2float %13 %int_3
%87 = OpLoad %v2float %85
%88 = OpCompositeExtract %float %87 0
%89 = OpAccessChain %_ptr_Uniform_v2float %13 %int_3
%90 = OpLoad %v2float %89
%91 = OpCompositeExtract %float %90 1
%92 = OpLoad %v4float %sk_FragCoord
%93 = OpCompositeExtract %float %92 1
%94 = OpFMul %float %91 %93
%95 = OpFAdd %float %88 %94
%96 = OpLoad %v4float %sk_FragCoord
%97 = OpCompositeExtract %float %96 2
%98 = OpLoad %v4float %sk_FragCoord
%99 = OpCompositeExtract %float %98 3
%100 = OpCompositeConstruct %v4float %83 %95 %97 %99
%101 = OpVectorShuffle %v2float %100 %100 0 0
%81 = OpDPdx %v2float %101
%80 = OpExtInst %v2float %1 FSign %81
%104 = OpFOrdEqual %v2bool %80 %103
%105 = OpAll %bool %104
OpBranch %79
%79 = OpLabel
%106 = OpPhi %bool %false %69 %105 %78
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%111 = OpLoad %v4float %sk_FragCoord
%112 = OpCompositeExtract %float %111 0
%113 = OpAccessChain %_ptr_Uniform_v2float %13 %int_3
%114 = OpLoad %v2float %113
%115 = OpCompositeExtract %float %114 0
%116 = OpAccessChain %_ptr_Uniform_v2float %13 %int_3
%117 = OpLoad %v2float %116
%118 = OpCompositeExtract %float %117 1
%119 = OpLoad %v4float %sk_FragCoord
%120 = OpCompositeExtract %float %119 1
%121 = OpFMul %float %118 %120
%122 = OpFAdd %float %115 %121
%123 = OpLoad %v4float %sk_FragCoord
%124 = OpCompositeExtract %float %123 2
%125 = OpLoad %v4float %sk_FragCoord
%126 = OpCompositeExtract %float %125 3
%127 = OpCompositeConstruct %v4float %112 %122 %124 %126
%128 = OpVectorShuffle %v2float %127 %127 1 1
%110 = OpDPdx %v2float %128
%109 = OpExtInst %v2float %1 FSign %110
%129 = OpFOrdEqual %v2bool %109 %22
%130 = OpAll %bool %129
OpBranch %108
%108 = OpLabel
%131 = OpPhi %bool %false %79 %130 %107
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%136 = OpLoad %v4float %sk_FragCoord
%137 = OpCompositeExtract %float %136 0
%138 = OpAccessChain %_ptr_Uniform_v2float %13 %int_3
%139 = OpLoad %v2float %138
%140 = OpCompositeExtract %float %139 0
%141 = OpAccessChain %_ptr_Uniform_v2float %13 %int_3
%142 = OpLoad %v2float %141
%143 = OpCompositeExtract %float %142 1
%144 = OpLoad %v4float %sk_FragCoord
%145 = OpCompositeExtract %float %144 1
%146 = OpFMul %float %143 %145
%147 = OpFAdd %float %140 %146
%148 = OpLoad %v4float %sk_FragCoord
%149 = OpCompositeExtract %float %148 2
%150 = OpLoad %v4float %sk_FragCoord
%151 = OpCompositeExtract %float %150 3
%152 = OpCompositeConstruct %v4float %137 %147 %149 %151
%135 = OpDPdx %v2float %152
%134 = OpExtInst %v2float %1 FSign %135
%154 = OpFOrdEqual %v2bool %134 %153
%155 = OpAll %bool %154
OpBranch %133
%133 = OpLabel
%156 = OpPhi %bool %false %108 %155 %132
OpSelectionMerge %160 None
OpBranchConditional %156 %158 %159
%158 = OpLabel
%161 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%163 = OpLoad %v4float %161
OpStore %157 %163
OpBranch %160
%159 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %13 %int_2
%166 = OpLoad %v4float %164
OpStore %157 %166
OpBranch %160
%160 = OpLabel
%167 = OpLoad %v4float %157
OpReturnValue %167
OpFunctionEnd

1 error
