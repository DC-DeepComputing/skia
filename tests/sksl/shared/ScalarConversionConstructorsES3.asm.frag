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
OpName %f "f"
OpName %i "i"
OpName %u "u"
OpName %b "b"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %i1 "i1"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpName %u1 "u1"
OpName %u2 "u2"
OpName %u3 "u3"
OpName %u4 "u4"
OpName %b1 "b1"
OpName %b2 "b2"
OpName %b3 "b3"
OpName %b4 "b4"
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
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
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
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_bool = OpTypePointer Function %bool
%float_1 = OpConstant %float 1
%int_1 = OpConstant %int 1
%uint_1 = OpConstant %uint 1
%uint_0 = OpConstant %uint 0
%float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
%f = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%u = OpVariable %_ptr_Function_uint Function
%b = OpVariable %_ptr_Function_bool Function
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%i1 = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_int Function
%i3 = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_int Function
%u1 = OpVariable %_ptr_Function_uint Function
%u2 = OpVariable %_ptr_Function_uint Function
%u3 = OpVariable %_ptr_Function_uint Function
%u4 = OpVariable %_ptr_Function_uint Function
%b1 = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_bool Function
%b3 = OpVariable %_ptr_Function_bool Function
%b4 = OpVariable %_ptr_Function_bool Function
%148 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
%35 = OpCompositeExtract %float %34 1
OpStore %f %35
%38 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%39 = OpLoad %v4float %38
%40 = OpCompositeExtract %float %39 1
%41 = OpConvertFToS %int %40
OpStore %i %41
%45 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%46 = OpLoad %v4float %45
%47 = OpCompositeExtract %float %46 1
%48 = OpConvertFToU %uint %47
OpStore %u %48
%51 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%52 = OpLoad %v4float %51
%53 = OpCompositeExtract %float %52 1
%54 = OpFUnordNotEqual %bool %53 %float_0
OpStore %b %54
%56 = OpLoad %float %f
OpStore %f1 %56
%58 = OpLoad %int %i
%59 = OpConvertSToF %float %58
OpStore %f2 %59
%61 = OpLoad %uint %u
%62 = OpConvertUToF %float %61
OpStore %f3 %62
%64 = OpLoad %bool %b
%65 = OpSelect %float %64 %float_1 %float_0
OpStore %f4 %65
%68 = OpLoad %float %f
%69 = OpConvertFToS %int %68
OpStore %i1 %69
%71 = OpLoad %int %i
OpStore %i2 %71
%73 = OpLoad %uint %u
%74 = OpBitcast %int %73
OpStore %i3 %74
%76 = OpLoad %bool %b
%77 = OpSelect %int %76 %int_1 %int_0
OpStore %i4 %77
%80 = OpLoad %float %f
%81 = OpConvertFToU %uint %80
OpStore %u1 %81
%83 = OpLoad %int %i
%84 = OpBitcast %uint %83
OpStore %u2 %84
%86 = OpLoad %uint %u
OpStore %u3 %86
%88 = OpLoad %bool %b
%89 = OpSelect %uint %88 %uint_1 %uint_0
OpStore %u4 %89
%93 = OpLoad %float %f
%94 = OpFUnordNotEqual %bool %93 %float_0
OpStore %b1 %94
%96 = OpLoad %int %i
%97 = OpINotEqual %bool %96 %int_0
OpStore %b2 %97
%99 = OpLoad %uint %u
%100 = OpINotEqual %bool %99 %uint_0
OpStore %b3 %100
%102 = OpLoad %bool %b
OpStore %b4 %102
%103 = OpLoad %float %f1
%104 = OpLoad %float %f2
%105 = OpFAdd %float %103 %104
%106 = OpLoad %float %f3
%107 = OpFAdd %float %105 %106
%108 = OpLoad %float %f4
%109 = OpFAdd %float %107 %108
%110 = OpLoad %int %i1
%111 = OpConvertSToF %float %110
%112 = OpFAdd %float %109 %111
%113 = OpLoad %int %i2
%114 = OpConvertSToF %float %113
%115 = OpFAdd %float %112 %114
%116 = OpLoad %int %i3
%117 = OpConvertSToF %float %116
%118 = OpFAdd %float %115 %117
%119 = OpLoad %int %i4
%120 = OpConvertSToF %float %119
%121 = OpFAdd %float %118 %120
%122 = OpLoad %uint %u1
%123 = OpConvertUToF %float %122
%124 = OpFAdd %float %121 %123
%125 = OpLoad %uint %u2
%126 = OpConvertUToF %float %125
%127 = OpFAdd %float %124 %126
%128 = OpLoad %uint %u3
%129 = OpConvertUToF %float %128
%130 = OpFAdd %float %127 %129
%131 = OpLoad %uint %u4
%132 = OpConvertUToF %float %131
%133 = OpFAdd %float %130 %132
%134 = OpLoad %bool %b1
%135 = OpSelect %float %134 %float_1 %float_0
%136 = OpFAdd %float %133 %135
%137 = OpLoad %bool %b2
%138 = OpSelect %float %137 %float_1 %float_0
%139 = OpFAdd %float %136 %138
%140 = OpLoad %bool %b3
%141 = OpSelect %float %140 %float_1 %float_0
%142 = OpFAdd %float %139 %141
%143 = OpLoad %bool %b4
%144 = OpSelect %float %143 %float_1 %float_0
%145 = OpFAdd %float %142 %144
%147 = OpFOrdEqual %bool %145 %float_16
OpSelectionMerge %152 None
OpBranchConditional %147 %150 %151
%150 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%154 = OpLoad %v4float %153
OpStore %148 %154
OpBranch %152
%151 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%156 = OpLoad %v4float %155
OpStore %148 %156
OpBranch %152
%152 = OpLabel
%157 = OpLoad %v4float %148
OpReturnValue %157
OpFunctionEnd
