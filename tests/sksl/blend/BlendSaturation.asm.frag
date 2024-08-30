               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %blend_color_saturation_Qhh3 "blend_color_saturation_Qhh3"
               OpName %blend_hslc_h4h2h4h4 "blend_hslc_h4h2h4h4"
               OpName %alpha "alpha"
               OpName %sda "sda"
               OpName %dsa "dsa"
               OpName %l "l"
               OpName %r "r"
               OpName %_2_mn "_2_mn"
               OpName %_3_mx "_3_mx"
               OpName %_4_lum "_4_lum"
               OpName %_5_result "_5_result"
               OpName %_6_minComp "_6_minComp"
               OpName %_7_maxComp "_7_maxComp"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %alpha RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %sda RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %dsa RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %l RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %r RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %_2_mn RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %_3_mx RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %_4_lum RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %_5_result RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %_6_minComp RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %_7_maxComp RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %14 = OpTypeFunction %float %_ptr_Function_v3float
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %37 = OpTypeFunction %v4float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v4float
%_ptr_Function_float = OpTypePointer Function %float
    %float_0 = OpConstant %float 0
       %bool = OpTypeBool
    %float_1 = OpConstant %float 1
        %109 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%float_0_300000012 = OpConstant %float 0.300000012
%float_0_589999974 = OpConstant %float 0.589999974
%float_0_109999999 = OpConstant %float 0.109999999
        %116 = OpConstantComposite %v3float %float_0_300000012 %float_0_589999974 %float_0_109999999
      %false = OpConstantFalse %bool
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
       %void = OpTypeVoid
        %187 = OpTypeFunction %void
        %189 = OpConstantComposite %v2float %float_1 %float_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%blend_color_saturation_Qhh3 = OpFunction %float None %14
         %15 = OpFunctionParameter %_ptr_Function_v3float
         %16 = OpLabel
         %19 = OpLoad %v3float %15
         %20 = OpCompositeExtract %float %19 0
         %21 = OpLoad %v3float %15
         %22 = OpCompositeExtract %float %21 1
         %18 = OpExtInst %float %1 FMax %20 %22
         %23 = OpLoad %v3float %15
         %24 = OpCompositeExtract %float %23 2
         %17 = OpExtInst %float %1 FMax %18 %24
         %27 = OpLoad %v3float %15
         %28 = OpCompositeExtract %float %27 0
         %29 = OpLoad %v3float %15
         %30 = OpCompositeExtract %float %29 1
         %26 = OpExtInst %float %1 FMin %28 %30
         %31 = OpLoad %v3float %15
         %32 = OpCompositeExtract %float %31 2
         %25 = OpExtInst %float %1 FMin %26 %32
         %33 = OpFSub %float %17 %25
               OpReturnValue %33
               OpFunctionEnd
%blend_hslc_h4h2h4h4 = OpFunction %v4float None %37
         %38 = OpFunctionParameter %_ptr_Function_v2float
         %39 = OpFunctionParameter %_ptr_Function_v4float
         %40 = OpFunctionParameter %_ptr_Function_v4float
         %41 = OpLabel
      %alpha = OpVariable %_ptr_Function_float Function
        %sda = OpVariable %_ptr_Function_v3float Function
        %dsa = OpVariable %_ptr_Function_v3float Function
          %l = OpVariable %_ptr_Function_v3float Function
         %67 = OpVariable %_ptr_Function_v3float Function
          %r = OpVariable %_ptr_Function_v3float Function
         %76 = OpVariable %_ptr_Function_v3float Function
      %_2_mn = OpVariable %_ptr_Function_float Function
      %_3_mx = OpVariable %_ptr_Function_float Function
         %96 = OpVariable %_ptr_Function_v3float Function
        %102 = OpVariable %_ptr_Function_v3float Function
     %_4_lum = OpVariable %_ptr_Function_float Function
  %_5_result = OpVariable %_ptr_Function_v3float Function
 %_6_minComp = OpVariable %_ptr_Function_float Function
 %_7_maxComp = OpVariable %_ptr_Function_float Function
         %44 = OpLoad %v4float %40
         %45 = OpCompositeExtract %float %44 3
         %46 = OpLoad %v4float %39
         %47 = OpCompositeExtract %float %46 3
         %48 = OpFMul %float %45 %47
               OpStore %alpha %48
         %50 = OpLoad %v4float %39
         %51 = OpVectorShuffle %v3float %50 %50 0 1 2
         %52 = OpLoad %v4float %40
         %53 = OpCompositeExtract %float %52 3
         %54 = OpVectorTimesScalar %v3float %51 %53
               OpStore %sda %54
         %56 = OpLoad %v4float %40
         %57 = OpVectorShuffle %v3float %56 %56 0 1 2
         %58 = OpLoad %v4float %39
         %59 = OpCompositeExtract %float %58 3
         %60 = OpVectorTimesScalar %v3float %57 %59
               OpStore %dsa %60
         %62 = OpLoad %v2float %38
         %63 = OpCompositeExtract %float %62 0
         %64 = OpFUnordNotEqual %bool %63 %float_0
               OpSelectionMerge %70 None
               OpBranchConditional %64 %68 %69
         %68 = OpLabel
               OpStore %67 %60
               OpBranch %70
         %69 = OpLabel
               OpStore %67 %54
               OpBranch %70
         %70 = OpLabel
         %71 = OpLoad %v3float %67
               OpStore %l %71
         %73 = OpLoad %v2float %38
         %74 = OpCompositeExtract %float %73 0
         %75 = OpFUnordNotEqual %bool %74 %float_0
               OpSelectionMerge %79 None
               OpBranchConditional %75 %77 %78
         %77 = OpLabel
               OpStore %76 %54
               OpBranch %79
         %78 = OpLabel
               OpStore %76 %60
               OpBranch %79
         %79 = OpLabel
         %80 = OpLoad %v3float %76
               OpStore %r %80
         %81 = OpLoad %v2float %38
         %82 = OpCompositeExtract %float %81 1
         %83 = OpFUnordNotEqual %bool %82 %float_0
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
         %89 = OpCompositeExtract %float %71 0
         %90 = OpCompositeExtract %float %71 1
         %88 = OpExtInst %float %1 FMin %89 %90
         %91 = OpCompositeExtract %float %71 2
         %87 = OpExtInst %float %1 FMin %88 %91
               OpStore %_2_mn %87
         %94 = OpExtInst %float %1 FMax %89 %90
         %93 = OpExtInst %float %1 FMax %94 %91
               OpStore %_3_mx %93
         %95 = OpFOrdGreaterThan %bool %93 %87
               OpSelectionMerge %99 None
               OpBranchConditional %95 %97 %98
         %97 = OpLabel
        %100 = OpCompositeConstruct %v3float %87 %87 %87
        %101 = OpFSub %v3float %71 %100
               OpStore %102 %80
        %103 = OpFunctionCall %float %blend_color_saturation_Qhh3 %102
        %104 = OpVectorTimesScalar %v3float %101 %103
        %105 = OpFSub %float %93 %87
        %107 = OpFDiv %float %float_1 %105
        %108 = OpVectorTimesScalar %v3float %104 %107
               OpStore %96 %108
               OpBranch %99
         %98 = OpLabel
               OpStore %96 %109
               OpBranch %99
         %99 = OpLabel
        %110 = OpLoad %v3float %96
               OpStore %l %110
               OpStore %r %60
               OpBranch %85
         %85 = OpLabel
        %117 = OpLoad %v3float %r
        %112 = OpDot %float %116 %117
               OpStore %_4_lum %112
        %120 = OpLoad %v3float %l
        %119 = OpDot %float %116 %120
        %121 = OpFSub %float %112 %119
        %122 = OpLoad %v3float %l
        %123 = OpCompositeConstruct %v3float %121 %121 %121
        %124 = OpFAdd %v3float %123 %122
               OpStore %_5_result %124
        %128 = OpCompositeExtract %float %124 0
        %129 = OpCompositeExtract %float %124 1
        %127 = OpExtInst %float %1 FMin %128 %129
        %130 = OpCompositeExtract %float %124 2
        %126 = OpExtInst %float %1 FMin %127 %130
               OpStore %_6_minComp %126
        %133 = OpExtInst %float %1 FMax %128 %129
        %132 = OpExtInst %float %1 FMax %133 %130
               OpStore %_7_maxComp %132
        %135 = OpFOrdLessThan %bool %126 %float_0
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %138 = OpFUnordNotEqual %bool %112 %126
               OpBranch %137
        %137 = OpLabel
        %139 = OpPhi %bool %false %85 %138 %136
               OpSelectionMerge %141 None
               OpBranchConditional %139 %140 %141
        %140 = OpLabel
        %142 = OpCompositeConstruct %v3float %112 %112 %112
        %143 = OpFSub %v3float %124 %142
        %144 = OpFSub %float %112 %126
        %146 = OpFAdd %float %144 %float_6_10351562en05
        %147 = OpFDiv %float %112 %146
        %148 = OpVectorTimesScalar %v3float %143 %147
        %149 = OpFAdd %v3float %142 %148
               OpStore %_5_result %149
               OpBranch %141
        %141 = OpLabel
        %150 = OpFOrdGreaterThan %bool %132 %48
               OpSelectionMerge %152 None
               OpBranchConditional %150 %151 %152
        %151 = OpLabel
        %153 = OpFUnordNotEqual %bool %132 %112
               OpBranch %152
        %152 = OpLabel
        %154 = OpPhi %bool %false %141 %153 %151
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %157 = OpLoad %v3float %_5_result
        %158 = OpCompositeConstruct %v3float %112 %112 %112
        %159 = OpFSub %v3float %157 %158
        %160 = OpFSub %float %48 %112
        %161 = OpVectorTimesScalar %v3float %159 %160
        %162 = OpFSub %float %132 %112
        %163 = OpFAdd %float %162 %float_6_10351562en05
        %164 = OpFDiv %float %float_1 %163
        %165 = OpVectorTimesScalar %v3float %161 %164
        %166 = OpFAdd %v3float %158 %165
               OpStore %_5_result %166
               OpBranch %156
        %156 = OpLabel
        %167 = OpLoad %v3float %_5_result
        %168 = OpLoad %v4float %40
        %169 = OpVectorShuffle %v3float %168 %168 0 1 2
        %170 = OpFAdd %v3float %167 %169
        %171 = OpFSub %v3float %170 %60
        %172 = OpLoad %v4float %39
        %173 = OpVectorShuffle %v3float %172 %172 0 1 2
        %174 = OpFAdd %v3float %171 %173
        %175 = OpFSub %v3float %174 %54
        %176 = OpCompositeExtract %float %175 0
        %177 = OpCompositeExtract %float %175 1
        %178 = OpCompositeExtract %float %175 2
        %179 = OpLoad %v4float %39
        %180 = OpCompositeExtract %float %179 3
        %181 = OpLoad %v4float %40
        %182 = OpCompositeExtract %float %181 3
        %183 = OpFAdd %float %180 %182
        %184 = OpFSub %float %183 %48
        %185 = OpCompositeConstruct %v4float %176 %177 %178 %184
               OpReturnValue %185
               OpFunctionEnd
       %main = OpFunction %void None %187
        %188 = OpLabel
        %190 = OpVariable %_ptr_Function_v2float Function
        %196 = OpVariable %_ptr_Function_v4float Function
        %200 = OpVariable %_ptr_Function_v4float Function
               OpStore %190 %189
        %191 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
        %195 = OpLoad %v4float %191
               OpStore %196 %195
        %197 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
        %199 = OpLoad %v4float %197
               OpStore %200 %199
        %201 = OpFunctionCall %v4float %blend_hslc_h4h2h4h4 %190 %196 %200
               OpStore %sk_FragColor %201
               OpReturn
               OpFunctionEnd
