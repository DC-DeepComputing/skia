#include "src/sksl/codegen/SkSLSPIRVValidator.h"

static bool validate_spirv(ErrorReporter& reporter, std::string_view program, bool disassemble) {
    SkASSERT(0 == program.size() % 4);
    const uint32_t* programData = reinterpret_cast<const uint32_t*>(program.data());
    size_t programSize = program.size() / 4;

    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_0);
    std::string errors;
    auto msgFn = [&errors](spv_message_level_t, const char*, const spv_position_t&, const char* m) {
        errors += "SPIR-V validation error: ";
        errors += m;
        errors += '\n';
    };
    tools.SetMessageConsumer(msgFn);

    // Verify that the SPIR-V we produced is valid. At runtime, we will abort() with a message
    // explaining the error. In standalone mode (skslc), we will send the message, plus the
    // entire disassembled SPIR-V (for easier context & debugging) as *our* error message.
    bool result = tools.Validate(programData, programSize);
    if (!result) {
        if (disassemble) {
            // Convert the string-stream to a SPIR-V disassembly.
            std::string disassembly;
            uint32_t options = spvtools::SpirvTools::kDefaultDisassembleOption;
            options |= SPV_BINARY_TO_TEXT_OPTION_INDENT;
            if (tools.Disassemble(programData, programSize, &disassembly, options)) {
                errors.append(disassembly);
            }
            reporter.error(Position(), errors);
        } else {
            SkDEBUGFAILF("%s", errors.c_str());
        }
    }
    return result;
}

bool ValidateSPIRV(ErrorReporter& reporter, std::string_view program) {
    return validate_spirv(reporter, program, false);
}

bool ValidateSPIRVAndDissassemble(ErrorReporter& reporter, std::string_view program) {
    return validate_spirv(reporter, program, true);
}