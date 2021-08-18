OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragCoord %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragCoord "sk_FragCoord"
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %result "result"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
OpDecorate %sk_FragCoord BuiltIn FragCoord
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %161 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
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
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_6 = OpConstant %float 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
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
%result = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_mat2v2float Function
%b = OpVariable %_ptr_Function_mat2v2float Function
%c = OpVariable %_ptr_Function_mat3v3float Function
%d = OpVariable %_ptr_Function_mat3v3float Function
%e = OpVariable %_ptr_Function_mat4v4float Function
%f = OpVariable %_ptr_Function_mat2v2float Function
%155 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%36 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%37 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%38 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%34 = OpCompositeConstruct %mat3v3float %36 %37 %38
%41 = OpCompositeExtract %v3float %34 0
%42 = OpVectorShuffle %v2float %41 %41 0 1
%43 = OpCompositeExtract %v3float %34 1
%44 = OpVectorShuffle %v2float %43 %43 0 1
%40 = OpCompositeConstruct %mat2v2float %42 %44
OpStore %a %40
%45 = OpLoad %float %result
%48 = OpAccessChain %_ptr_Function_v2float %a %int_0
%49 = OpLoad %v2float %48
%50 = OpCompositeExtract %float %49 0
%51 = OpFAdd %float %45 %50
OpStore %result %51
%54 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%55 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%56 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%57 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%53 = OpCompositeConstruct %mat4v4float %54 %55 %56 %57
%60 = OpCompositeExtract %v4float %53 0
%61 = OpVectorShuffle %v2float %60 %60 0 1
%62 = OpCompositeExtract %v4float %53 1
%63 = OpVectorShuffle %v2float %62 %62 0 1
%59 = OpCompositeConstruct %mat2v2float %61 %63
OpStore %b %59
%64 = OpLoad %float %result
%65 = OpAccessChain %_ptr_Function_v2float %b %int_0
%66 = OpLoad %v2float %65
%67 = OpCompositeExtract %float %66 0
%68 = OpFAdd %float %64 %67
OpStore %result %68
%72 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%73 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%74 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%75 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%71 = OpCompositeConstruct %mat4v4float %72 %73 %74 %75
%77 = OpCompositeExtract %v4float %71 0
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%79 = OpCompositeExtract %v4float %71 1
%80 = OpVectorShuffle %v3float %79 %79 0 1 2
%81 = OpCompositeExtract %v4float %71 2
%82 = OpVectorShuffle %v3float %81 %81 0 1 2
%76 = OpCompositeConstruct %mat3v3float %78 %80 %82
OpStore %c %76
%83 = OpLoad %float %result
%84 = OpAccessChain %_ptr_Function_v3float %c %int_0
%86 = OpLoad %v3float %84
%87 = OpCompositeExtract %float %86 0
%88 = OpFAdd %float %83 %87
OpStore %result %88
%91 = OpCompositeConstruct %v2float %float_1 %float_0
%92 = OpCompositeConstruct %v2float %float_0 %float_1
%90 = OpCompositeConstruct %mat2v2float %91 %92
%94 = OpCompositeExtract %v2float %90 0
%95 = OpCompositeConstruct %v3float %94 %float_0
%96 = OpCompositeExtract %v2float %90 1
%97 = OpCompositeConstruct %v3float %96 %float_0
%98 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%93 = OpCompositeConstruct %mat3v3float %95 %97 %98
OpStore %d %93
%99 = OpLoad %float %result
%100 = OpAccessChain %_ptr_Function_v3float %d %int_0
%101 = OpLoad %v3float %100
%102 = OpCompositeExtract %float %101 0
%103 = OpFAdd %float %99 %102
OpStore %result %103
%107 = OpCompositeConstruct %v2float %float_1 %float_0
%108 = OpCompositeConstruct %v2float %float_0 %float_1
%106 = OpCompositeConstruct %mat2v2float %107 %108
%110 = OpCompositeExtract %v2float %106 0
%111 = OpCompositeConstruct %v3float %110 %float_0
%112 = OpCompositeExtract %v2float %106 1
%113 = OpCompositeConstruct %v3float %112 %float_0
%114 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%109 = OpCompositeConstruct %mat3v3float %111 %113 %114
%116 = OpCompositeExtract %v3float %109 0
%117 = OpCompositeConstruct %v4float %116 %float_0
%118 = OpCompositeExtract %v3float %109 1
%119 = OpCompositeConstruct %v4float %118 %float_0
%120 = OpCompositeExtract %v3float %109 2
%121 = OpCompositeConstruct %v4float %120 %float_0
%122 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%115 = OpCompositeConstruct %mat4v4float %117 %119 %121 %122
OpStore %e %115
%123 = OpLoad %float %result
%124 = OpAccessChain %_ptr_Function_v4float %e %int_0
%126 = OpLoad %v4float %124
%127 = OpCompositeExtract %float %126 0
%128 = OpFAdd %float %123 %127
OpStore %result %128
%131 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%132 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%133 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%134 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%130 = OpCompositeConstruct %mat4v4float %131 %132 %133 %134
%136 = OpCompositeExtract %v4float %130 0
%137 = OpVectorShuffle %v3float %136 %136 0 1 2
%138 = OpCompositeExtract %v4float %130 1
%139 = OpVectorShuffle %v3float %138 %138 0 1 2
%140 = OpCompositeExtract %v4float %130 2
%141 = OpVectorShuffle %v3float %140 %140 0 1 2
%135 = OpCompositeConstruct %mat3v3float %137 %139 %141
%143 = OpCompositeExtract %v3float %135 0
%144 = OpVectorShuffle %v2float %143 %143 0 1
%145 = OpCompositeExtract %v3float %135 1
%146 = OpVectorShuffle %v2float %145 %145 0 1
%142 = OpCompositeConstruct %mat2v2float %144 %146
OpStore %f %142
%147 = OpLoad %float %result
%148 = OpAccessChain %_ptr_Function_v2float %f %int_0
%149 = OpLoad %v2float %148
%150 = OpCompositeExtract %float %149 0
%151 = OpFAdd %float %147 %150
OpStore %result %151
%152 = OpLoad %float %result
%154 = OpFOrdEqual %bool %152 %float_6
OpSelectionMerge %158 None
OpBranchConditional %154 %156 %157
%156 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%161 = OpLoad %v4float %159
OpStore %155 %161
OpBranch %158
%157 = OpLabel
%162 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%164 = OpLoad %v4float %162
OpStore %155 %164
OpBranch %158
%158 = OpLabel
%165 = OpLoad %v4float %155
OpReturnValue %165
OpFunctionEnd
