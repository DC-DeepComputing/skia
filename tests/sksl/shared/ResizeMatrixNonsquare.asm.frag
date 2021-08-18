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
OpName %g "g"
OpName %h "h"
OpName %i "i"
OpName %j "j"
OpName %k "k"
OpName %l "l"
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
OpDecorate %166 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_1 = OpConstant %float 1
%mat2v3float = OpTypeMatrix %v3float 2
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%mat3v2float = OpTypeMatrix %v2float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%mat4v2float = OpTypeMatrix %v2float 4
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v4float = OpTypeMatrix %v4float 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
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
%g = OpVariable %_ptr_Function_mat3v3float Function
%h = OpVariable %_ptr_Function_mat3v3float Function
%i = OpVariable %_ptr_Function_mat4v4float Function
%j = OpVariable %_ptr_Function_mat4v4float Function
%k = OpVariable %_ptr_Function_mat2v4float Function
%l = OpVariable %_ptr_Function_mat4v2float Function
%160 = OpVariable %_ptr_Function_v4float Function
OpStore %result %float_0
%36 = OpCompositeConstruct %v3float %float_1 %float_0 %float_0
%37 = OpCompositeConstruct %v3float %float_0 %float_1 %float_0
%35 = OpCompositeConstruct %mat2v3float %36 %37
%40 = OpCompositeExtract %v3float %35 0
%41 = OpCompositeExtract %v3float %35 1
%42 = OpCompositeConstruct %v3float %float_0 %float_0 %float_1
%39 = OpCompositeConstruct %mat3v3float %40 %41 %42
OpStore %g %39
%43 = OpLoad %float %result
%46 = OpAccessChain %_ptr_Function_v3float %g %int_0
%48 = OpLoad %v3float %46
%49 = OpCompositeExtract %float %48 0
%50 = OpFAdd %float %43 %49
OpStore %result %50
%53 = OpCompositeConstruct %v2float %float_1 %float_0
%54 = OpCompositeConstruct %v2float %float_0 %float_1
%55 = OpCompositeConstruct %v2float %float_0 %float_0
%52 = OpCompositeConstruct %mat3v2float %53 %54 %55
%58 = OpCompositeExtract %v2float %52 0
%59 = OpCompositeConstruct %v3float %58 %float_0
%60 = OpCompositeExtract %v2float %52 1
%61 = OpCompositeConstruct %v3float %60 %float_0
%62 = OpCompositeExtract %v2float %52 2
%63 = OpCompositeConstruct %v3float %62 %float_1
%57 = OpCompositeConstruct %mat3v3float %59 %61 %63
OpStore %h %57
%64 = OpLoad %float %result
%65 = OpAccessChain %_ptr_Function_v3float %h %int_0
%66 = OpLoad %v3float %65
%67 = OpCompositeExtract %float %66 0
%68 = OpFAdd %float %64 %67
OpStore %result %68
%73 = OpCompositeConstruct %v2float %float_1 %float_0
%74 = OpCompositeConstruct %v2float %float_0 %float_1
%75 = OpCompositeConstruct %v2float %float_0 %float_0
%76 = OpCompositeConstruct %v2float %float_0 %float_0
%72 = OpCompositeConstruct %mat4v2float %73 %74 %75 %76
%79 = OpCompositeExtract %v2float %72 0
%80 = OpCompositeConstruct %v3float %79 %float_0
%81 = OpCompositeExtract %v2float %72 1
%82 = OpCompositeConstruct %v3float %81 %float_0
%83 = OpCompositeExtract %v2float %72 2
%84 = OpCompositeConstruct %v3float %83 %float_1
%85 = OpCompositeExtract %v2float %72 3
%86 = OpCompositeConstruct %v3float %85 %float_0
%78 = OpCompositeConstruct %mat4v3float %80 %82 %84 %86
%89 = OpCompositeExtract %v3float %78 0
%90 = OpCompositeConstruct %v4float %89 %float_0
%91 = OpCompositeExtract %v3float %78 1
%92 = OpCompositeConstruct %v4float %91 %float_0
%93 = OpCompositeExtract %v3float %78 2
%94 = OpCompositeConstruct %v4float %93 %float_0
%95 = OpCompositeExtract %v3float %78 3
%96 = OpCompositeConstruct %v4float %95 %float_1
%88 = OpCompositeConstruct %mat4v4float %90 %92 %94 %96
OpStore %i %88
%97 = OpLoad %float %result
%98 = OpAccessChain %_ptr_Function_v4float %i %int_0
%100 = OpLoad %v4float %98
%101 = OpCompositeExtract %float %100 0
%102 = OpFAdd %float %97 %101
OpStore %result %102
%105 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%106 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%104 = OpCompositeConstruct %mat2v4float %105 %106
%109 = OpCompositeExtract %v4float %104 0
%110 = OpCompositeExtract %v4float %104 1
%111 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%108 = OpCompositeConstruct %mat3v4float %109 %110 %111
%114 = OpCompositeExtract %v4float %108 0
%115 = OpCompositeExtract %v4float %108 1
%116 = OpCompositeExtract %v4float %108 2
%117 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%113 = OpCompositeConstruct %mat4v4float %114 %115 %116 %117
OpStore %j %113
%118 = OpLoad %float %result
%119 = OpAccessChain %_ptr_Function_v4float %j %int_0
%120 = OpLoad %v4float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpFAdd %float %118 %121
OpStore %result %122
%126 = OpCompositeConstruct %v2float %float_1 %float_0
%127 = OpCompositeConstruct %v2float %float_0 %float_1
%128 = OpCompositeConstruct %v2float %float_0 %float_0
%129 = OpCompositeConstruct %v2float %float_0 %float_0
%125 = OpCompositeConstruct %mat4v2float %126 %127 %128 %129
%131 = OpCompositeExtract %v2float %125 0
%132 = OpCompositeConstruct %v4float %131 %float_0 %float_0
%133 = OpCompositeExtract %v2float %125 1
%134 = OpCompositeConstruct %v4float %133 %float_0 %float_0
%130 = OpCompositeConstruct %mat2v4float %132 %134
OpStore %k %130
%135 = OpLoad %float %result
%136 = OpAccessChain %_ptr_Function_v4float %k %int_0
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 0
%139 = OpFAdd %float %135 %138
OpStore %result %139
%143 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%144 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%142 = OpCompositeConstruct %mat2v4float %143 %144
%146 = OpCompositeExtract %v4float %142 0
%147 = OpVectorShuffle %v2float %146 %146 0 1
%148 = OpCompositeExtract %v4float %142 1
%149 = OpVectorShuffle %v2float %148 %148 0 1
%150 = OpCompositeConstruct %v2float %float_0 %float_0
%151 = OpCompositeConstruct %v2float %float_0 %float_0
%145 = OpCompositeConstruct %mat4v2float %147 %149 %150 %151
OpStore %l %145
%152 = OpLoad %float %result
%153 = OpAccessChain %_ptr_Function_v2float %l %int_0
%154 = OpLoad %v2float %153
%155 = OpCompositeExtract %float %154 0
%156 = OpFAdd %float %152 %155
OpStore %result %156
%157 = OpLoad %float %result
%159 = OpFOrdEqual %bool %157 %float_6
OpSelectionMerge %163 None
OpBranchConditional %159 %161 %162
%161 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%166 = OpLoad %v4float %164
OpStore %160 %166
OpBranch %163
%162 = OpLabel
%167 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%169 = OpLoad %v4float %167
OpStore %160 %169
OpBranch %163
%163 = OpLabel
%170 = OpLoad %v4float %160
OpReturnValue %170
OpFunctionEnd
