### Compilation failed:

error: SPIR-V validation error: Expected Image to be of type OpTypeImage
  %22 = OpImageQuerySize %v2uint %23

               OpCapability ImageQuery
               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %t "t"                        ; id %11
               OpName %main "main"                  ; id %6
               OpName %dims "dims"                  ; id %18

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %t RelaxedPrecision
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %23 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %12 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %13 = OpTypeSampledImage %12
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
          %t = OpVariable %_ptr_UniformConstant_13 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
    %v2float = OpTypeVector %float 2


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
       %dims =   OpVariable %_ptr_Function_v2uint Function
         %23 =   OpLoad %13 %t                      ; RelaxedPrecision
         %22 =   OpImageQuerySize %v2uint %23
                 OpStore %dims %22
         %25 =   OpLoad %13 %t                      ; RelaxedPrecision
         %26 =   OpCompositeExtract %uint %22 0
         %27 =   OpConvertUToF %float %26           ; RelaxedPrecision
         %28 =   OpCompositeExtract %uint %22 1
         %29 =   OpConvertUToF %float %28           ; RelaxedPrecision
         %31 =   OpCompositeConstruct %v2float %27 %29  ; RelaxedPrecision
         %24 =   OpImageSampleImplicitLod %v4float %25 %31  ; RelaxedPrecision
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd

1 error
