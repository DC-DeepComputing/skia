               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %soft_light_component_Qhh2h2 "soft_light_component_Qhh2h2"
               OpName %DSqd "DSqd"
               OpName %DCub "DCub"
               OpName %DaSqd "DaSqd"
               OpName %DaCub "DaCub"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
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
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %DSqd RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %DCub RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %DaSqd RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %DaCub RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
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
               OpDecorate %146 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
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
               OpDecorate %182 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %13 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
%float_6_10351562en05 = OpConstant %float 6.10351562e-05
    %float_1 = OpConstant %float 1
    %float_4 = OpConstant %float 4
%_ptr_Function_float = OpTypePointer Function %float
    %float_3 = OpConstant %float 3
    %float_6 = OpConstant %float 6
   %float_12 = OpConstant %float 12
   %float_16 = OpConstant %float 16
       %void = OpTypeVoid
        %176 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
    %float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_0 = OpConstant %int 0
%soft_light_component_Qhh2h2 = OpFunction %float None %13
         %14 = OpFunctionParameter %_ptr_Function_v2float
         %15 = OpFunctionParameter %_ptr_Function_v2float
         %16 = OpLabel
       %DSqd = OpVariable %_ptr_Function_float Function
       %DCub = OpVariable %_ptr_Function_float Function
      %DaSqd = OpVariable %_ptr_Function_float Function
      %DaCub = OpVariable %_ptr_Function_float Function
         %18 = OpLoad %v2float %14
         %19 = OpCompositeExtract %float %18 0
         %20 = OpFMul %float %float_2 %19
         %21 = OpLoad %v2float %14
         %22 = OpCompositeExtract %float %21 1
         %23 = OpFOrdLessThanEqual %bool %20 %22
               OpSelectionMerge %27 None
               OpBranchConditional %23 %25 %26
         %25 = OpLabel
         %28 = OpLoad %v2float %15
         %29 = OpCompositeExtract %float %28 0
         %30 = OpLoad %v2float %15
         %31 = OpCompositeExtract %float %30 0
         %32 = OpFMul %float %29 %31
         %33 = OpLoad %v2float %14
         %34 = OpCompositeExtract %float %33 1
         %35 = OpLoad %v2float %14
         %36 = OpCompositeExtract %float %35 0
         %37 = OpFMul %float %float_2 %36
         %38 = OpFSub %float %34 %37
         %39 = OpFMul %float %32 %38
         %40 = OpLoad %v2float %15
         %41 = OpCompositeExtract %float %40 1
         %43 = OpFAdd %float %41 %float_6_10351562en05
         %44 = OpFDiv %float %39 %43
         %46 = OpLoad %v2float %15
         %47 = OpCompositeExtract %float %46 1
         %48 = OpFSub %float %float_1 %47
         %49 = OpLoad %v2float %14
         %50 = OpCompositeExtract %float %49 0
         %51 = OpFMul %float %48 %50
         %52 = OpFAdd %float %44 %51
         %53 = OpLoad %v2float %15
         %54 = OpCompositeExtract %float %53 0
         %55 = OpLoad %v2float %14
         %56 = OpCompositeExtract %float %55 1
         %57 = OpFNegate %float %56
         %58 = OpLoad %v2float %14
         %59 = OpCompositeExtract %float %58 0
         %60 = OpFMul %float %float_2 %59
         %61 = OpFAdd %float %57 %60
         %62 = OpFAdd %float %61 %float_1
         %63 = OpFMul %float %54 %62
         %64 = OpFAdd %float %52 %63
               OpReturnValue %64
         %26 = OpLabel
         %66 = OpLoad %v2float %15
         %67 = OpCompositeExtract %float %66 0
         %68 = OpFMul %float %float_4 %67
         %69 = OpLoad %v2float %15
         %70 = OpCompositeExtract %float %69 1
         %71 = OpFOrdLessThanEqual %bool %68 %70
               OpSelectionMerge %74 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %77 = OpLoad %v2float %15
         %78 = OpCompositeExtract %float %77 0
         %79 = OpLoad %v2float %15
         %80 = OpCompositeExtract %float %79 0
         %81 = OpFMul %float %78 %80
               OpStore %DSqd %81
         %83 = OpLoad %v2float %15
         %84 = OpCompositeExtract %float %83 0
         %85 = OpFMul %float %81 %84
               OpStore %DCub %85
         %87 = OpLoad %v2float %15
         %88 = OpCompositeExtract %float %87 1
         %89 = OpLoad %v2float %15
         %90 = OpCompositeExtract %float %89 1
         %91 = OpFMul %float %88 %90
               OpStore %DaSqd %91
         %93 = OpLoad %v2float %15
         %94 = OpCompositeExtract %float %93 1
         %95 = OpFMul %float %91 %94
               OpStore %DaCub %95
         %96 = OpLoad %v2float %14
         %97 = OpCompositeExtract %float %96 0
         %98 = OpLoad %v2float %15
         %99 = OpCompositeExtract %float %98 0
        %101 = OpLoad %v2float %14
        %102 = OpCompositeExtract %float %101 1
        %103 = OpFMul %float %float_3 %102
        %105 = OpLoad %v2float %14
        %106 = OpCompositeExtract %float %105 0
        %107 = OpFMul %float %float_6 %106
        %108 = OpFSub %float %103 %107
        %109 = OpFSub %float %108 %float_1
        %110 = OpFMul %float %99 %109
        %111 = OpFSub %float %97 %110
        %112 = OpFMul %float %91 %111
        %114 = OpLoad %v2float %15
        %115 = OpCompositeExtract %float %114 1
        %116 = OpFMul %float %float_12 %115
        %117 = OpFMul %float %116 %81
        %118 = OpLoad %v2float %14
        %119 = OpCompositeExtract %float %118 1
        %120 = OpLoad %v2float %14
        %121 = OpCompositeExtract %float %120 0
        %122 = OpFMul %float %float_2 %121
        %123 = OpFSub %float %119 %122
        %124 = OpFMul %float %117 %123
        %125 = OpFAdd %float %112 %124
        %127 = OpFMul %float %float_16 %85
        %128 = OpLoad %v2float %14
        %129 = OpCompositeExtract %float %128 1
        %130 = OpLoad %v2float %14
        %131 = OpCompositeExtract %float %130 0
        %132 = OpFMul %float %float_2 %131
        %133 = OpFSub %float %129 %132
        %134 = OpFMul %float %127 %133
        %135 = OpFSub %float %125 %134
        %136 = OpLoad %v2float %14
        %137 = OpCompositeExtract %float %136 0
        %138 = OpFMul %float %95 %137
        %139 = OpFSub %float %135 %138
        %140 = OpFAdd %float %91 %float_6_10351562en05
        %141 = OpFDiv %float %139 %140
               OpReturnValue %141
         %73 = OpLabel
        %142 = OpLoad %v2float %15
        %143 = OpCompositeExtract %float %142 0
        %144 = OpLoad %v2float %14
        %145 = OpCompositeExtract %float %144 1
        %146 = OpLoad %v2float %14
        %147 = OpCompositeExtract %float %146 0
        %148 = OpFMul %float %float_2 %147
        %149 = OpFSub %float %145 %148
        %150 = OpFAdd %float %149 %float_1
        %151 = OpFMul %float %143 %150
        %152 = OpLoad %v2float %14
        %153 = OpCompositeExtract %float %152 0
        %154 = OpFAdd %float %151 %153
        %156 = OpLoad %v2float %15
        %157 = OpCompositeExtract %float %156 1
        %158 = OpLoad %v2float %15
        %159 = OpCompositeExtract %float %158 0
        %160 = OpFMul %float %157 %159
        %155 = OpExtInst %float %1 Sqrt %160
        %161 = OpLoad %v2float %14
        %162 = OpCompositeExtract %float %161 1
        %163 = OpLoad %v2float %14
        %164 = OpCompositeExtract %float %163 0
        %165 = OpFMul %float %float_2 %164
        %166 = OpFSub %float %162 %165
        %167 = OpFMul %float %155 %166
        %168 = OpFSub %float %154 %167
        %169 = OpLoad %v2float %15
        %170 = OpCompositeExtract %float %169 1
        %171 = OpLoad %v2float %14
        %172 = OpCompositeExtract %float %171 0
        %173 = OpFMul %float %170 %172
        %174 = OpFSub %float %168 %173
               OpReturnValue %174
         %74 = OpLabel
               OpBranch %27
         %27 = OpLabel
               OpUnreachable
               OpFunctionEnd
       %main = OpFunction %void None %176
        %177 = OpLabel
        %186 = OpVariable %_ptr_Function_v4float Function
        %197 = OpVariable %_ptr_Function_v2float Function
        %201 = OpVariable %_ptr_Function_v2float Function
        %206 = OpVariable %_ptr_Function_v2float Function
        %210 = OpVariable %_ptr_Function_v2float Function
        %215 = OpVariable %_ptr_Function_v2float Function
        %219 = OpVariable %_ptr_Function_v2float Function
        %178 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %182 = OpLoad %v4float %178
        %183 = OpCompositeExtract %float %182 3
        %185 = OpFOrdEqual %bool %183 %float_0
               OpSelectionMerge %190 None
               OpBranchConditional %185 %188 %189
        %188 = OpLabel
        %191 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %193 = OpLoad %v4float %191
               OpStore %186 %193
               OpBranch %190
        %189 = OpLabel
        %194 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %195 = OpLoad %v4float %194
        %196 = OpVectorShuffle %v2float %195 %195 0 3
               OpStore %197 %196
        %198 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %199 = OpLoad %v4float %198
        %200 = OpVectorShuffle %v2float %199 %199 0 3
               OpStore %201 %200
        %202 = OpFunctionCall %float %soft_light_component_Qhh2h2 %197 %201
        %203 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %204 = OpLoad %v4float %203
        %205 = OpVectorShuffle %v2float %204 %204 1 3
               OpStore %206 %205
        %207 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %208 = OpLoad %v4float %207
        %209 = OpVectorShuffle %v2float %208 %208 1 3
               OpStore %210 %209
        %211 = OpFunctionCall %float %soft_light_component_Qhh2h2 %206 %210
        %212 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %213 = OpLoad %v4float %212
        %214 = OpVectorShuffle %v2float %213 %213 2 3
               OpStore %215 %214
        %216 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %217 = OpLoad %v4float %216
        %218 = OpVectorShuffle %v2float %217 %217 2 3
               OpStore %219 %218
        %220 = OpFunctionCall %float %soft_light_component_Qhh2h2 %215 %219
        %221 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %222 = OpLoad %v4float %221
        %223 = OpCompositeExtract %float %222 3
        %224 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %225 = OpLoad %v4float %224
        %226 = OpCompositeExtract %float %225 3
        %227 = OpFSub %float %float_1 %226
        %228 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %229 = OpLoad %v4float %228
        %230 = OpCompositeExtract %float %229 3
        %231 = OpFMul %float %227 %230
        %232 = OpFAdd %float %223 %231
        %233 = OpCompositeConstruct %v4float %202 %211 %220 %232
               OpStore %186 %233
               OpBranch %190
        %190 = OpLabel
        %234 = OpLoad %v4float %186
               OpStore %sk_FragColor %234
               OpReturn
               OpFunctionEnd
