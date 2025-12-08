               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 4
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %23 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %int %uint           ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Uniform_int = OpTypePointer Uniform %int
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
      %v2int = OpTypeVector %int 2
    %v2float = OpTypeVector %float 2
     %v2uint = OpTypeVector %uint 2
      %v3int = OpTypeVector %int 3
    %v3float = OpTypeVector %float 3
     %v3uint = OpTypeVector %uint 3
      %v4int = OpTypeVector %int 4
     %v4uint = OpTypeVector %uint 4


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
         %19 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %22 =   OpLoad %int %19
         %18 =   OpExtInst %int %5 FindSMsb %22
         %23 =   OpConvertSToF %float %18           ; RelaxedPrecision
         %25 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %28 =   OpLoad %uint %25
         %24 =   OpExtInst %int %5 FindUMsb %28
         %29 =   OpConvertSToF %float %24           ; RelaxedPrecision
         %30 =   OpFAdd %float %23 %29              ; RelaxedPrecision
         %31 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %31 %30
         %34 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %35 =   OpLoad %int %34
         %37 =   OpCompositeConstruct %v2int %35 %35
         %33 =   OpExtInst %v2int %5 FindSMsb %37
         %38 =   OpCompositeExtract %int %33 0
         %39 =   OpConvertSToF %float %38           ; RelaxedPrecision
         %40 =   OpCompositeExtract %int %33 1
         %41 =   OpConvertSToF %float %40           ; RelaxedPrecision
         %43 =   OpCompositeConstruct %v2float %39 %41  ; RelaxedPrecision
         %45 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %46 =   OpLoad %uint %45
         %48 =   OpCompositeConstruct %v2uint %46 %46
         %44 =   OpExtInst %v2int %5 FindUMsb %48
         %49 =   OpCompositeExtract %int %44 0
         %50 =   OpConvertSToF %float %49           ; RelaxedPrecision
         %51 =   OpCompositeExtract %int %44 1
         %52 =   OpConvertSToF %float %51           ; RelaxedPrecision
         %53 =   OpCompositeConstruct %v2float %50 %52  ; RelaxedPrecision
         %54 =   OpFAdd %v2float %43 %53                ; RelaxedPrecision
         %55 =   OpLoad %v4float %sk_FragColor          ; RelaxedPrecision
         %56 =   OpVectorShuffle %v4float %55 %54 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %56
         %58 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %59 =   OpLoad %int %58
         %61 =   OpCompositeConstruct %v3int %59 %59 %59
         %57 =   OpExtInst %v3int %5 FindSMsb %61
         %62 =   OpCompositeExtract %int %57 0
         %63 =   OpConvertSToF %float %62           ; RelaxedPrecision
         %64 =   OpCompositeExtract %int %57 1
         %65 =   OpConvertSToF %float %64           ; RelaxedPrecision
         %66 =   OpCompositeExtract %int %57 2
         %67 =   OpConvertSToF %float %66           ; RelaxedPrecision
         %69 =   OpCompositeConstruct %v3float %63 %65 %67  ; RelaxedPrecision
         %71 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %72 =   OpLoad %uint %71
         %74 =   OpCompositeConstruct %v3uint %72 %72 %72
         %70 =   OpExtInst %v3int %5 FindUMsb %74
         %75 =   OpCompositeExtract %int %70 0
         %76 =   OpConvertSToF %float %75           ; RelaxedPrecision
         %77 =   OpCompositeExtract %int %70 1
         %78 =   OpConvertSToF %float %77           ; RelaxedPrecision
         %79 =   OpCompositeExtract %int %70 2
         %80 =   OpConvertSToF %float %79           ; RelaxedPrecision
         %81 =   OpCompositeConstruct %v3float %76 %78 %80  ; RelaxedPrecision
         %82 =   OpFAdd %v3float %69 %81                    ; RelaxedPrecision
         %83 =   OpLoad %v4float %sk_FragColor              ; RelaxedPrecision
         %84 =   OpVectorShuffle %v4float %83 %82 4 5 6 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %84
         %86 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %87 =   OpLoad %int %86
         %89 =   OpCompositeConstruct %v4int %87 %87 %87 %87
         %85 =   OpExtInst %v4int %5 FindSMsb %89
         %90 =   OpCompositeExtract %int %85 0
         %91 =   OpConvertSToF %float %90           ; RelaxedPrecision
         %92 =   OpCompositeExtract %int %85 1
         %93 =   OpConvertSToF %float %92           ; RelaxedPrecision
         %94 =   OpCompositeExtract %int %85 2
         %95 =   OpConvertSToF %float %94           ; RelaxedPrecision
         %96 =   OpCompositeExtract %int %85 3
         %97 =   OpConvertSToF %float %96           ; RelaxedPrecision
         %98 =   OpCompositeConstruct %v4float %91 %93 %95 %97  ; RelaxedPrecision
        %100 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
        %101 =   OpLoad %uint %100
        %103 =   OpCompositeConstruct %v4uint %101 %101 %101 %101
         %99 =   OpExtInst %v4int %5 FindUMsb %103
        %104 =   OpCompositeExtract %int %99 0
        %105 =   OpConvertSToF %float %104          ; RelaxedPrecision
        %106 =   OpCompositeExtract %int %99 1
        %107 =   OpConvertSToF %float %106          ; RelaxedPrecision
        %108 =   OpCompositeExtract %int %99 2
        %109 =   OpConvertSToF %float %108          ; RelaxedPrecision
        %110 =   OpCompositeExtract %int %99 3
        %111 =   OpConvertSToF %float %110          ; RelaxedPrecision
        %112 =   OpCompositeConstruct %v4float %105 %107 %109 %111  ; RelaxedPrecision
        %113 =   OpFAdd %v4float %98 %112                           ; RelaxedPrecision
                 OpStore %sk_FragColor %113
                 OpReturn
               OpFunctionEnd
