               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpName %z "z"
               OpName %b "b"
               OpName %c "c"
               OpName %d "d"
               OpName %e "e"
               OpName %f "f"
               OpName %w "w"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %137 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
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
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_3 = OpConstant %int 3
      %int_2 = OpConstant %int 2
      %int_4 = OpConstant %int 4
      %int_1 = OpConstant %int 1
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
    %float_4 = OpConstant %float 4
      %false = OpConstantFalse %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
   %float_12 = OpConstant %float 12
%float_0_100000001 = OpConstant %float 0.100000001
    %float_6 = OpConstant %float 6
      %int_0 = OpConstant %int 0
     %int_n1 = OpConstant %int -1
      %int_5 = OpConstant %int 5
      %int_6 = OpConstant %int 6
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
     %int_n6 = OpConstant %int -6
        %112 = OpConstantComposite %v2int %int_n6 %int_n6
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
          %x = OpVariable %_ptr_Function_float Function
          %y = OpVariable %_ptr_Function_float Function
          %z = OpVariable %_ptr_Function_int Function
          %b = OpVariable %_ptr_Function_bool Function
          %c = OpVariable %_ptr_Function_bool Function
          %d = OpVariable %_ptr_Function_bool Function
          %e = OpVariable %_ptr_Function_bool Function
          %f = OpVariable %_ptr_Function_bool Function
          %w = OpVariable %_ptr_Function_v2int Function
        %130 = OpVariable %_ptr_Function_v4float Function
               OpStore %x %float_1
               OpStore %y %float_2
               OpStore %z %int_3
         %32 = OpFSub %float %float_1 %float_1
         %33 = OpFMul %float %float_2 %float_1
         %34 = OpFMul %float %33 %float_1
         %35 = OpFSub %float %float_2 %float_1
         %36 = OpFMul %float %34 %35
         %37 = OpFAdd %float %32 %36
               OpStore %x %37
         %38 = OpFDiv %float %37 %float_2
         %39 = OpFDiv %float %38 %37
               OpStore %y %39
         %41 = OpSDiv %int %int_3 %int_2
         %42 = OpSMod %int %41 %int_3
         %44 = OpShiftLeftLogical %int %42 %int_4
         %45 = OpShiftRightArithmetic %int %44 %int_2
         %47 = OpShiftLeftLogical %int %45 %int_1
               OpStore %z %47
         %53 = OpFOrdGreaterThan %bool %37 %float_4
         %54 = OpFOrdLessThan %bool %37 %float_2
         %55 = OpLogicalEqual %bool %53 %54
               OpSelectionMerge %57 None
               OpBranchConditional %55 %57 %56
         %56 = OpLabel
         %59 = OpAccessChain %_ptr_Uniform_float %7 %int_2
         %61 = OpLoad %float %59
         %62 = OpFOrdGreaterThanEqual %bool %float_2 %61
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
         %65 = OpFOrdLessThanEqual %bool %39 %37
               OpBranch %64
         %64 = OpLabel
         %66 = OpPhi %bool %false %56 %65 %63
               OpBranch %57
         %57 = OpLabel
         %67 = OpPhi %bool %true %22 %66 %64
               OpStore %b %67
         %69 = OpAccessChain %_ptr_Uniform_float %7 %int_2
         %70 = OpLoad %float %69
         %71 = OpFOrdGreaterThan %bool %70 %float_2
               OpStore %c %71
         %73 = OpLogicalNotEqual %bool %67 %71
               OpStore %d %73
               OpSelectionMerge %76 None
               OpBranchConditional %67 %75 %76
         %75 = OpLabel
               OpBranch %76
         %76 = OpLabel
         %77 = OpPhi %bool %false %57 %71 %75
               OpStore %e %77
               OpSelectionMerge %80 None
               OpBranchConditional %67 %80 %79
         %79 = OpLabel
               OpBranch %80
         %80 = OpLabel
         %81 = OpPhi %bool %true %76 %71 %79
               OpStore %f %81
         %83 = OpFAdd %float %37 %float_12
               OpStore %x %83
         %84 = OpFSub %float %83 %float_12
               OpStore %x %84
         %86 = OpFMul %float %39 %float_0_100000001
               OpStore %y %86
         %87 = OpFMul %float %84 %86
               OpStore %x %87
               OpStore %x %float_6
         %89 = OpSelect %float %67 %float_1 %float_0
         %90 = OpSelect %float %71 %float_1 %float_0
         %91 = OpFMul %float %89 %90
         %92 = OpSelect %float %73 %float_1 %float_0
         %93 = OpFMul %float %91 %92
         %94 = OpSelect %float %77 %float_1 %float_0
         %95 = OpFMul %float %93 %94
         %96 = OpSelect %float %81 %float_1 %float_0
         %97 = OpFMul %float %95 %96
               OpStore %y %97
         %99 = OpBitwiseOr %int %47 %int_0
               OpStore %z %99
        %101 = OpBitwiseAnd %int %99 %int_n1
               OpStore %z %101
        %102 = OpBitwiseXor %int %101 %int_0
               OpStore %z %102
        %103 = OpShiftRightArithmetic %int %102 %int_2
               OpStore %z %103
        %104 = OpShiftLeftLogical %int %103 %int_4
               OpStore %z %104
        %106 = OpSMod %int %104 %int_5
               OpStore %z %106
               OpStore %y %float_6
               OpStore %z %int_6
               OpStore %w %112
        %113 = OpNot %v2int %112
               OpStore %w %113
        %114 = OpCompositeExtract %int %113 0
        %115 = OpIEqual %bool %114 %int_5
               OpSelectionMerge %117 None
               OpBranchConditional %115 %116 %117
        %116 = OpLabel
        %118 = OpCompositeExtract %int %113 1
        %119 = OpIEqual %bool %118 %int_5
               OpBranch %117
        %117 = OpLabel
        %120 = OpPhi %bool %false %80 %119 %116
               OpSelectionMerge %122 None
               OpBranchConditional %120 %121 %122
        %121 = OpLabel
               OpBranch %122
        %122 = OpLabel
        %123 = OpPhi %bool %false %117 %true %121
               OpSelectionMerge %125 None
               OpBranchConditional %123 %124 %125
        %124 = OpLabel
               OpBranch %125
        %125 = OpLabel
        %126 = OpPhi %bool %false %122 %true %124
               OpSelectionMerge %128 None
               OpBranchConditional %126 %127 %128
        %127 = OpLabel
               OpBranch %128
        %128 = OpLabel
        %129 = OpPhi %bool %false %125 %true %127
               OpSelectionMerge %134 None
               OpBranchConditional %129 %132 %133
        %132 = OpLabel
        %135 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %137 = OpLoad %v4float %135
               OpStore %130 %137
               OpBranch %134
        %133 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %139 = OpLoad %v4float %138
               OpStore %130 %139
               OpBranch %134
        %134 = OpLabel
        %140 = OpLoad %v4float %130
               OpReturnValue %140
               OpFunctionEnd
