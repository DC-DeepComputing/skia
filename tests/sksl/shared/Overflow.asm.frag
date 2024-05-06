               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %hugeH "hugeH"
               OpName %hugeF "hugeF"
               OpName %hugeI "hugeI"
               OpName %hugeU "hugeU"
               OpName %hugeS "hugeS"
               OpName %hugeUS "hugeUS"
               OpName %hugeNI "hugeNI"
               OpName %hugeNS "hugeNS"
               OpName %hugeIvec "hugeIvec"
               OpName %hugeUvec "hugeUvec"
               OpName %hugeMxM "hugeMxM"
               OpName %hugeMxV "hugeMxV"
               OpName %hugeVxM "hugeVxM"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %hugeH RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %hugeS RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %hugeUS RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %hugeNS RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_9_99999962e_35 = OpConstant %float 9.99999962e+35
%float_9_99999988e_26 = OpConstant %float 9.99999988e+26
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1073741824 = OpConstant %int 1073741824
%int_1048576 = OpConstant %int 1048576
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%uint_2147483648 = OpConstant %uint 2147483648
%uint_524288 = OpConstant %uint 524288
  %int_16384 = OpConstant %int 16384
      %int_8 = OpConstant %int 8
 %uint_32768 = OpConstant %uint 32768
     %uint_2 = OpConstant %uint 2
%int_n2147483648 = OpConstant %int -2147483648
 %int_524288 = OpConstant %int 524288
 %int_n32768 = OpConstant %int -32768
      %int_4 = OpConstant %int 4
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
         %68 = OpConstantComposite %v4int %int_1073741824 %int_1073741824 %int_1073741824 %int_1073741824
  %int_32768 = OpConstant %int 32768
         %70 = OpConstantComposite %v4int %int_32768 %int_32768 %int_32768 %int_32768
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
         %75 = OpConstantComposite %v4uint %uint_2147483648 %uint_2147483648 %uint_2147483648 %uint_2147483648
 %uint_16384 = OpConstant %uint 16384
         %77 = OpConstantComposite %v4uint %uint_16384 %uint_16384 %uint_16384 %uint_16384
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_1_00000002e_20 = OpConstant %float 1.00000002e+20
         %83 = OpConstantComposite %v4float %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20 %float_1_00000002e_20
         %84 = OpConstantComposite %mat4v4float %83 %83 %83 %83
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
        %128 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
        %129 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
      %hugeH = OpVariable %_ptr_Function_float Function
      %hugeF = OpVariable %_ptr_Function_float Function
      %hugeI = OpVariable %_ptr_Function_int Function
      %hugeU = OpVariable %_ptr_Function_uint Function
      %hugeS = OpVariable %_ptr_Function_int Function
     %hugeUS = OpVariable %_ptr_Function_uint Function
     %hugeNI = OpVariable %_ptr_Function_int Function
     %hugeNS = OpVariable %_ptr_Function_int Function
   %hugeIvec = OpVariable %_ptr_Function_v4int Function
   %hugeUvec = OpVariable %_ptr_Function_v4uint Function
    %hugeMxM = OpVariable %_ptr_Function_mat4v4float Function
    %hugeMxV = OpVariable %_ptr_Function_v4float Function
    %hugeVxM = OpVariable %_ptr_Function_v4float Function
         %26 = OpFMul %float %float_9_99999962e_35 %float_9_99999962e_35
         %27 = OpFMul %float %26 %float_9_99999962e_35
         %29 = OpFMul %float %27 %float_9_99999988e_26
               OpStore %hugeH %29
         %31 = OpFMul %float %float_9_99999962e_35 %float_9_99999962e_35
         %32 = OpFMul %float %31 %float_9_99999962e_35
         %33 = OpFMul %float %32 %float_9_99999988e_26
               OpStore %hugeF %33
         %39 = OpIMul %int %int_1073741824 %int_1048576
               OpStore %hugeI %39
         %45 = OpIMul %uint %uint_2147483648 %uint_524288
               OpStore %hugeU %45
         %48 = OpIMul %int %int_16384 %int_16384
         %50 = OpIMul %int %48 %int_8
               OpStore %hugeS %50
         %53 = OpIMul %uint %uint_32768 %uint_32768
         %55 = OpIMul %uint %53 %uint_2
               OpStore %hugeUS %55
         %59 = OpIMul %int %int_n2147483648 %int_524288
               OpStore %hugeNI %59
         %62 = OpIMul %int %int_n32768 %int_16384
         %64 = OpIMul %int %62 %int_4
               OpStore %hugeNS %64
         %71 = OpIMul %v4int %68 %70
               OpStore %hugeIvec %71
         %78 = OpIMul %v4uint %75 %77
               OpStore %hugeUvec %78
         %85 = OpMatrixTimesMatrix %mat4v4float %84 %84
               OpStore %hugeMxM %85
         %88 = OpMatrixTimesVector %v4float %84 %83
               OpStore %hugeMxV %88
         %90 = OpVectorTimesMatrix %v4float %83 %84
               OpStore %hugeVxM %90
         %91 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %94 = OpLoad %v4float %91
         %95 = OpExtInst %float %1 FClamp %29 %float_0 %float_1
         %97 = OpVectorTimesScalar %v4float %94 %95
         %98 = OpExtInst %float %1 FClamp %33 %float_0 %float_1
         %99 = OpVectorTimesScalar %v4float %97 %98
        %101 = OpConvertSToF %float %39
        %100 = OpExtInst %float %1 FClamp %101 %float_0 %float_1
        %102 = OpVectorTimesScalar %v4float %99 %100
        %104 = OpConvertUToF %float %45
        %103 = OpExtInst %float %1 FClamp %104 %float_0 %float_1
        %105 = OpVectorTimesScalar %v4float %102 %103
        %107 = OpConvertSToF %float %50
        %106 = OpExtInst %float %1 FClamp %107 %float_0 %float_1
        %108 = OpVectorTimesScalar %v4float %105 %106
        %110 = OpConvertUToF %float %55
        %109 = OpExtInst %float %1 FClamp %110 %float_0 %float_1
        %111 = OpVectorTimesScalar %v4float %108 %109
        %113 = OpConvertSToF %float %59
        %112 = OpExtInst %float %1 FClamp %113 %float_0 %float_1
        %114 = OpVectorTimesScalar %v4float %111 %112
        %116 = OpConvertSToF %float %64
        %115 = OpExtInst %float %1 FClamp %116 %float_0 %float_1
        %117 = OpVectorTimesScalar %v4float %114 %115
        %119 = OpCompositeExtract %int %71 0
        %120 = OpConvertSToF %float %119
        %121 = OpCompositeExtract %int %71 1
        %122 = OpConvertSToF %float %121
        %123 = OpCompositeExtract %int %71 2
        %124 = OpConvertSToF %float %123
        %125 = OpCompositeExtract %int %71 3
        %126 = OpConvertSToF %float %125
        %127 = OpCompositeConstruct %v4float %120 %122 %124 %126
        %118 = OpExtInst %v4float %1 FClamp %127 %128 %129
        %130 = OpFMul %v4float %117 %118
        %132 = OpCompositeExtract %uint %78 0
        %133 = OpConvertUToF %float %132
        %134 = OpCompositeExtract %uint %78 1
        %135 = OpConvertUToF %float %134
        %136 = OpCompositeExtract %uint %78 2
        %137 = OpConvertUToF %float %136
        %138 = OpCompositeExtract %uint %78 3
        %139 = OpConvertUToF %float %138
        %140 = OpCompositeConstruct %v4float %133 %135 %137 %139
        %131 = OpExtInst %v4float %1 FClamp %140 %128 %129
        %141 = OpFMul %v4float %130 %131
        %143 = OpAccessChain %_ptr_Function_v4float %hugeMxM %int_0
        %144 = OpLoad %v4float %143
        %142 = OpExtInst %v4float %1 FClamp %144 %128 %129
        %145 = OpFMul %v4float %141 %142
        %146 = OpExtInst %v4float %1 FClamp %88 %128 %129
        %147 = OpFMul %v4float %145 %146
        %148 = OpExtInst %v4float %1 FClamp %90 %128 %129
        %149 = OpFMul %v4float %147 %148
               OpReturnValue %149
               OpFunctionEnd
