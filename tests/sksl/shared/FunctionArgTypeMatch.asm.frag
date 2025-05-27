               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %104 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
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
         %95 = OpVariable %_ptr_Function_v4float Function
               OpSelectionMerge %27 None
               OpBranchConditional %true %26 %27
         %26 = OpLabel
               OpBranch %27
         %27 = OpLabel
         %28 = OpPhi %bool %false %22 %true %26
               OpSelectionMerge %30 None
               OpBranchConditional %28 %29 %30
         %29 = OpLabel
               OpBranch %30
         %30 = OpLabel
         %31 = OpPhi %bool %false %27 %true %29
               OpSelectionMerge %33 None
               OpBranchConditional %31 %32 %33
         %32 = OpLabel
               OpBranch %33
         %33 = OpLabel
         %34 = OpPhi %bool %false %30 %true %32
               OpSelectionMerge %36 None
               OpBranchConditional %34 %35 %36
         %35 = OpLabel
               OpBranch %36
         %36 = OpLabel
         %37 = OpPhi %bool %false %33 %true %35
               OpSelectionMerge %39 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
               OpBranch %39
         %39 = OpLabel
         %40 = OpPhi %bool %false %36 %true %38
               OpSelectionMerge %42 None
               OpBranchConditional %40 %41 %42
         %41 = OpLabel
               OpBranch %42
         %42 = OpLabel
         %43 = OpPhi %bool %false %39 %true %41
               OpSelectionMerge %45 None
               OpBranchConditional %43 %44 %45
         %44 = OpLabel
               OpBranch %45
         %45 = OpLabel
         %46 = OpPhi %bool %false %42 %true %44
               OpSelectionMerge %48 None
               OpBranchConditional %46 %47 %48
         %47 = OpLabel
               OpBranch %48
         %48 = OpLabel
         %49 = OpPhi %bool %false %45 %true %47
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
               OpBranch %51
         %51 = OpLabel
         %52 = OpPhi %bool %false %48 %true %50
               OpSelectionMerge %54 None
               OpBranchConditional %52 %53 %54
         %53 = OpLabel
               OpBranch %54
         %54 = OpLabel
         %55 = OpPhi %bool %false %51 %true %53
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
               OpBranch %57
         %57 = OpLabel
         %58 = OpPhi %bool %false %54 %true %56
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
               OpBranch %60
         %60 = OpLabel
         %61 = OpPhi %bool %false %57 %true %59
               OpSelectionMerge %63 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
               OpBranch %63
         %63 = OpLabel
         %64 = OpPhi %bool %false %60 %true %62
               OpSelectionMerge %66 None
               OpBranchConditional %64 %65 %66
         %65 = OpLabel
               OpBranch %66
         %66 = OpLabel
         %67 = OpPhi %bool %false %63 %true %65
               OpSelectionMerge %69 None
               OpBranchConditional %67 %68 %69
         %68 = OpLabel
               OpBranch %69
         %69 = OpLabel
         %70 = OpPhi %bool %false %66 %true %68
               OpSelectionMerge %72 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
               OpBranch %72
         %72 = OpLabel
         %73 = OpPhi %bool %false %69 %true %71
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
               OpBranch %75
         %75 = OpLabel
         %76 = OpPhi %bool %false %72 %true %74
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
               OpBranch %78
         %78 = OpLabel
         %79 = OpPhi %bool %false %75 %true %77
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
               OpBranch %81
         %81 = OpLabel
         %82 = OpPhi %bool %false %78 %true %80
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
               OpBranch %84
         %84 = OpLabel
         %85 = OpPhi %bool %false %81 %true %83
               OpSelectionMerge %87 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
               OpBranch %87
         %87 = OpLabel
         %88 = OpPhi %bool %false %84 %true %86
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
               OpBranch %90
         %90 = OpLabel
         %91 = OpPhi %bool %false %87 %true %89
               OpSelectionMerge %93 None
               OpBranchConditional %91 %92 %93
         %92 = OpLabel
               OpBranch %93
         %93 = OpLabel
         %94 = OpPhi %bool %false %90 %true %92
               OpSelectionMerge %99 None
               OpBranchConditional %94 %97 %98
         %97 = OpLabel
        %100 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %104 = OpLoad %v4float %100
               OpStore %95 %104
               OpBranch %99
         %98 = OpLabel
        %105 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %107 = OpLoad %v4float %105
               OpStore %95 %107
               OpBranch %99
         %99 = OpLabel
        %108 = OpLoad %v4float %95
               OpReturnValue %108
               OpFunctionEnd
