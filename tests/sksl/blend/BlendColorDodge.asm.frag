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
OpName %color_dodge_component_Qhh2h2 "color_dodge_component_Qhh2h2"
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
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
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
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
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
%108 = OpTypeFunction %void
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
%color_dodge_component_Qhh2h2 = OpFunction %float None %33
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%87 = OpVariable %_ptr_Function_float Function
%88 = OpVariable %_ptr_Function_float Function
%37 = OpLoad %v2float %35
%38 = OpCompositeExtract %float %37 0
%39 = OpFOrdEqual %bool %38 %float_0
OpSelectionMerge %42 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%43 = OpLoad %v2float %34
%44 = OpCompositeExtract %float %43 0
%46 = OpLoad %v2float %35
%47 = OpCompositeExtract %float %46 1
%48 = OpFSub %float %float_1 %47
%49 = OpFMul %float %44 %48
OpReturnValue %49
%41 = OpLabel
%51 = OpLoad %v2float %34
%52 = OpCompositeExtract %float %51 1
%53 = OpLoad %v2float %34
%54 = OpCompositeExtract %float %53 0
%55 = OpFSub %float %52 %54
OpStore %delta %55
%56 = OpFOrdEqual %bool %55 %float_0
OpSelectionMerge %59 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpLoad %v2float %34
%61 = OpCompositeExtract %float %60 1
%62 = OpLoad %v2float %35
%63 = OpCompositeExtract %float %62 1
%64 = OpFMul %float %61 %63
%65 = OpLoad %v2float %34
%66 = OpCompositeExtract %float %65 0
%67 = OpLoad %v2float %35
%68 = OpCompositeExtract %float %67 1
%69 = OpFSub %float %float_1 %68
%70 = OpFMul %float %66 %69
%71 = OpFAdd %float %64 %70
%72 = OpLoad %v2float %35
%73 = OpCompositeExtract %float %72 0
%74 = OpLoad %v2float %34
%75 = OpCompositeExtract %float %74 1
%76 = OpFSub %float %float_1 %75
%77 = OpFMul %float %73 %76
%78 = OpFAdd %float %71 %77
OpReturnValue %78
%58 = OpLabel
%80 = OpLoad %v2float %35
%81 = OpCompositeExtract %float %80 1
%82 = OpLoad %v2float %35
%83 = OpCompositeExtract %float %82 0
%84 = OpLoad %v2float %34
%85 = OpCompositeExtract %float %84 1
%86 = OpFMul %float %83 %85
OpStore %87 %86
OpStore %88 %55
%89 = OpFunctionCall %float %guarded_divide_Qhhh %87 %88
%79 = OpExtInst %float %1 FMin %81 %89
OpStore %delta %79
%90 = OpLoad %v2float %34
%91 = OpCompositeExtract %float %90 1
%92 = OpFMul %float %79 %91
%93 = OpLoad %v2float %34
%94 = OpCompositeExtract %float %93 0
%95 = OpLoad %v2float %35
%96 = OpCompositeExtract %float %95 1
%97 = OpFSub %float %float_1 %96
%98 = OpFMul %float %94 %97
%99 = OpFAdd %float %92 %98
%100 = OpLoad %v2float %35
%101 = OpCompositeExtract %float %100 0
%102 = OpLoad %v2float %34
%103 = OpCompositeExtract %float %102 1
%104 = OpFSub %float %float_1 %103
%105 = OpFMul %float %101 %104
%106 = OpFAdd %float %99 %105
OpReturnValue %106
%59 = OpLabel
OpBranch %42
%42 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %108
%109 = OpLabel
%116 = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v2float Function
%126 = OpVariable %_ptr_Function_v2float Function
%130 = OpVariable %_ptr_Function_v2float Function
%135 = OpVariable %_ptr_Function_v2float Function
%139 = OpVariable %_ptr_Function_v2float Function
%10 = OpSelect %float %false %float_9_99999994en09 %float_0
OpStore %_kGuardedDivideEpsilon %10
%110 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%114 = OpLoad %v4float %110
%115 = OpVectorShuffle %v2float %114 %114 0 3
OpStore %116 %115
%117 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%119 = OpLoad %v4float %117
%120 = OpVectorShuffle %v2float %119 %119 0 3
OpStore %121 %120
%122 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %116 %121
%123 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%124 = OpLoad %v4float %123
%125 = OpVectorShuffle %v2float %124 %124 1 3
OpStore %126 %125
%127 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v2float %128 %128 1 3
OpStore %130 %129
%131 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %126 %130
%132 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%133 = OpLoad %v4float %132
%134 = OpVectorShuffle %v2float %133 %133 2 3
OpStore %135 %134
%136 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%137 = OpLoad %v4float %136
%138 = OpVectorShuffle %v2float %137 %137 2 3
OpStore %139 %138
%140 = OpFunctionCall %float %color_dodge_component_Qhh2h2 %135 %139
%141 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%142 = OpLoad %v4float %141
%143 = OpCompositeExtract %float %142 3
%144 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
%145 = OpLoad %v4float %144
%146 = OpCompositeExtract %float %145 3
%147 = OpFSub %float %float_1 %146
%148 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 3
%151 = OpFMul %float %147 %150
%152 = OpFAdd %float %143 %151
%153 = OpCompositeConstruct %v4float %122 %131 %140 %152
OpStore %sk_FragColor %153
OpReturn
OpFunctionEnd
