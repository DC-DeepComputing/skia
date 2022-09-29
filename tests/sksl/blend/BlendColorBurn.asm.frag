OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %_kGuardedDivideEpsilon "$kGuardedDivideEpsilon"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %guarded_divide_Qhhh "guarded_divide_Qhhh"
OpName %color_burn_component_Qhh2h2 "color_burn_component_Qhh2h2"
OpName %delta "delta"
OpName %main "main"
OpDecorate %_kGuardedDivideEpsilon RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %18 Binding 0
OpDecorate %18 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
%float = OpTypeFloat 32
%_ptr_Private_float = OpTypePointer Private %float
%_kGuardedDivideEpsilon = OpVariable %_ptr_Private_float Private
%bool = OpTypeBool
%false = OpConstantFalse %bool
%float_9_99999994en09 = OpConstant %float 9.99999994e-09
%float_0 = OpConstant %float 0
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_float = OpTypePointer Function %float
%22 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%113 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%guarded_divide_Qhhh = OpFunction %float None %22
%23 = OpFunctionParameter %_ptr_Function_float
%24 = OpFunctionParameter %_ptr_Function_float
%25 = OpLabel
%26 = OpLoad %float %23
%27 = OpLoad %float %24
%28 = OpLoad %float %_kGuardedDivideEpsilon
%29 = OpFAdd %float %27 %28
%30 = OpFDiv %float %26 %29
OpReturnValue %30
OpFunctionEnd
%color_burn_component_Qhh2h2 = OpFunction %float None %33
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%89 = OpVariable %_ptr_Function_float Function
%92 = OpVariable %_ptr_Function_float Function
%37 = OpLoad %v2float %35
%38 = OpCompositeExtract %float %37 1
%39 = OpLoad %v2float %35
%40 = OpCompositeExtract %float %39 0
%41 = OpFOrdEqual %bool %38 %40
OpSelectionMerge %44 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpLoad %v2float %34
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %35
%48 = OpCompositeExtract %float %47 1
%49 = OpFMul %float %46 %48
%50 = OpLoad %v2float %34
%51 = OpCompositeExtract %float %50 0
%53 = OpLoad %v2float %35
%54 = OpCompositeExtract %float %53 1
%55 = OpFSub %float %float_1 %54
%56 = OpFMul %float %51 %55
%57 = OpFAdd %float %49 %56
%58 = OpLoad %v2float %35
%59 = OpCompositeExtract %float %58 0
%60 = OpLoad %v2float %34
%61 = OpCompositeExtract %float %60 1
%62 = OpFSub %float %float_1 %61
%63 = OpFMul %float %59 %62
%64 = OpFAdd %float %57 %63
OpReturnValue %64
%43 = OpLabel
%65 = OpLoad %v2float %34
%66 = OpCompositeExtract %float %65 0
%67 = OpFOrdEqual %bool %66 %float_0
OpSelectionMerge %70 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpLoad %v2float %35
%72 = OpCompositeExtract %float %71 0
%73 = OpLoad %v2float %34
%74 = OpCompositeExtract %float %73 1
%75 = OpFSub %float %float_1 %74
%76 = OpFMul %float %72 %75
OpReturnValue %76
%69 = OpLabel
%79 = OpLoad %v2float %35
%80 = OpCompositeExtract %float %79 1
%81 = OpLoad %v2float %35
%82 = OpCompositeExtract %float %81 1
%83 = OpLoad %v2float %35
%84 = OpCompositeExtract %float %83 0
%85 = OpFSub %float %82 %84
%86 = OpLoad %v2float %34
%87 = OpCompositeExtract %float %86 1
%88 = OpFMul %float %85 %87
OpStore %89 %88
%90 = OpLoad %v2float %34
%91 = OpCompositeExtract %float %90 0
OpStore %92 %91
%93 = OpFunctionCall %float %guarded_divide_Qhhh %89 %92
%94 = OpFSub %float %80 %93
%78 = OpExtInst %float %1 FMax %float_0 %94
OpStore %delta %78
%95 = OpLoad %v2float %34
%96 = OpCompositeExtract %float %95 1
%97 = OpFMul %float %78 %96
%98 = OpLoad %v2float %34
%99 = OpCompositeExtract %float %98 0
%100 = OpLoad %v2float %35
%101 = OpCompositeExtract %float %100 1
%102 = OpFSub %float %float_1 %101
%103 = OpFMul %float %99 %102
%104 = OpFAdd %float %97 %103
%105 = OpLoad %v2float %35
%106 = OpCompositeExtract %float %105 0
%107 = OpLoad %v2float %34
%108 = OpCompositeExtract %float %107 1
%109 = OpFSub %float %float_1 %108
%110 = OpFMul %float %106 %109
%111 = OpFAdd %float %104 %110
OpReturnValue %111
%70 = OpLabel
OpBranch %44
%44 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %113
%114 = OpLabel
%121 = OpVariable %_ptr_Function_v2float Function
%126 = OpVariable %_ptr_Function_v2float Function
%131 = OpVariable %_ptr_Function_v2float Function
%135 = OpVariable %_ptr_Function_v2float Function
%140 = OpVariable %_ptr_Function_v2float Function
%144 = OpVariable %_ptr_Function_v2float Function
%10 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_kGuardedDivideEpsilon %10
%115 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%119 = OpLoad %v4float %115
%120 = OpVectorShuffle %v2float %119 %119 0 3
OpStore %121 %120
%122 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%124 = OpLoad %v4float %122
%125 = OpVectorShuffle %v2float %124 %124 0 3
OpStore %126 %125
%127 = OpFunctionCall %float %color_burn_component_Qhh2h2 %121 %126
%128 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%129 = OpLoad %v4float %128
%130 = OpVectorShuffle %v2float %129 %129 1 3
OpStore %131 %130
%132 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%133 = OpLoad %v4float %132
%134 = OpVectorShuffle %v2float %133 %133 1 3
OpStore %135 %134
%136 = OpFunctionCall %float %color_burn_component_Qhh2h2 %131 %135
%137 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%138 = OpLoad %v4float %137
%139 = OpVectorShuffle %v2float %138 %138 2 3
OpStore %140 %139
%141 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%142 = OpLoad %v4float %141
%143 = OpVectorShuffle %v2float %142 %142 2 3
OpStore %144 %143
%145 = OpFunctionCall %float %color_burn_component_Qhh2h2 %140 %144
%146 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%147 = OpLoad %v4float %146
%148 = OpCompositeExtract %float %147 3
%149 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%150 = OpLoad %v4float %149
%151 = OpCompositeExtract %float %150 3
%152 = OpFSub %float %float_1 %151
%153 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 3
%156 = OpFMul %float %152 %155
%157 = OpFAdd %float %148 %156
%158 = OpCompositeConstruct %v4float %127 %136 %145 %157
OpStore %sk_FragColor %158
OpReturn
OpFunctionEnd
