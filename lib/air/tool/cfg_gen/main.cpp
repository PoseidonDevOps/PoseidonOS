
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "src/config/ConfigInterface.h"

int
main(void)
{
    std::cout << "\n config generating files...\n";

    // air.h gen start

    std::string air_h = AIR_H_GEN;
    std::ofstream air_h_file(air_h.data());

    if (false == air_h_file.is_open())
    {
        std::cout << " config generation failed.\n\n";
        return -1;
    }

    air_h_file << "// generated by config module\n\n";
    air_h_file << "#ifndef AIR_H\n";
    air_h_file << "#define AIR_H\n\n";

    air_h_file << "#include <stdint.h>\n";
    air_h_file << "#include <stdbool.h>\n\n";
    air_h_file << "#include \"src/lib/CType.h\"\n\n";

    air_h_file << "enum AIR_NODE\n";
    air_h_file << "{\n";

    try
    {
        uint32_t node_size = cfg::GetArrSize(config::ConfigType::NODE);
        for (uint32_t i = 0; i < node_size; i++)
        {
            air_h_file << "    " << cfg::GetName(config::ConfigType::NODE, i) << ",\n";
        }
        air_h_file << "};\n\n";

        air_h_file << "#ifdef __cplusplus\n";
        air_h_file << "extern \"C\" {\n";
        air_h_file << "#endif\n\n";

        air_h_file << "#define AIRLOG(NID, AID, VAL1, VAL2) \\\n";
        air_h_file << "    switch(NID) { \\\n";
        for (uint32_t i = 0; i < node_size; i++)
        {
            air_h_file << "    case " << cfg::GetName(config::ConfigType::NODE, i)
                       << ": AIR_" << cfg::GetName(config::ConfigType::NODE, i)
                       << "(AID, VAL1, VAL2); break; \\\n";
        }
        air_h_file << "    default: AIR_DONOTHING(); break; \\\n";
        air_h_file << "    }\n\n";

        air_h_file << "void AIR_INITIALIZE(uint32_t cpu_num);\n";
        air_h_file << "void AIR_ACTIVATE(void);\n";
        air_h_file << "void AIR_DEACTIVATE(void);\n";
        air_h_file << "void AIR_FINALIZE(void);\n\n";

        for (uint32_t i = 0; i < node_size; i++)
        {
            air_h_file << "void AIR_" << cfg::GetName(config::ConfigType::NODE, i)
                       << "(uint32_t aid, uint32_t value1, uint64_t value2);\n";
        }
    }
    catch (const std::out_of_range& e)
    {
        std::cout << e.what();
    }
    air_h_file << "\n";

    air_h_file << "void AIR_DONOTHING(void);\n\n";

    air_h_file << "#ifdef __cplusplus\n";
    air_h_file << "}\n";
    air_h_file << "#endif\n\n";

    air_h_file << "#endif // AIR_H";

    air_h_file.close();
    std::cout << " - Air.h\n";

    // air.h gen end

    // air.cpp gen start

    std::string air_cpp = AIR_CPP_GEN;
    std::ofstream air_cpp_file(air_cpp.data());

    if (false == air_cpp_file.is_open())
    {
        std::cout << " config generation failed.\n\n";
        return -1;
    }

    air_cpp_file << "// generated by config module\n\n";
    air_cpp_file << "#include \"src/api/Air.h\"\n";
    air_cpp_file << "#include \"src/api/AirTemplate.h\"\n\n";

    air_cpp_file << "extern \"C\"\n";
    air_cpp_file << "void AIR_INITIALIZE(uint32_t cpu_num) {\n";
    air_cpp_file << "    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, \"AirBuild\"), true>::Initialize(cpu_num);\n";
    air_cpp_file << "}\n\n";

    air_cpp_file << "extern \"C\"\n";
    air_cpp_file << "void AIR_ACTIVATE(void) {\n";
    air_cpp_file << "    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, \"AirBuild\"), true>::Activate();\n";
    air_cpp_file << "}\n\n";

    air_cpp_file << "extern \"C\"\n";
    air_cpp_file << "void AIR_DEACTIVATE(void) {\n";
    air_cpp_file << "    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, \"AirBuild\"), true>::Deactivate();\n";
    air_cpp_file << "}\n\n";

    air_cpp_file << "extern \"C\"\n";
    air_cpp_file << "void AIR_FINALIZE(void) {\n";
    air_cpp_file << "    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, \"AirBuild\"), true>::Finalize();\n";
    air_cpp_file << "}\n\n";

    try
    {
        uint32_t node_size = cfg::GetArrSize(config::ConfigType::NODE);
        for (uint32_t i = 0; i < node_size; i++)
        {
            air_cpp_file << "extern \"C\"\n";
            air_cpp_file << "void AIR_" << cfg::GetName(config::ConfigType::NODE, i)
                         << "(uint32_t aid, uint32_t value1, uint64_t value2) {\n";
            air_cpp_file << "    AIR<cfg::GetIntValue(config::ConfigType::DEFAULT, \"AirBuild\"),\n";
            air_cpp_file << "    cfg::GetIntValue(config::ConfigType::NODE, \"NodeBuild\", \"" << cfg::GetName(config::ConfigType::NODE, i) << "\")>\n";
            air_cpp_file << "    ::LogData<cfg::GetIndex(config::ConfigType::NODE, \"" << cfg::GetName(config::ConfigType::NODE, i) << "\")>(aid, value1, value2);\n";
            air_cpp_file << "}\n\n";
        }
    }
    catch (const std::out_of_range& e)
    {
        std::cout << e.what();
    }

    air_cpp_file << "extern \"C\"\n";
    air_cpp_file << "void AIR_DONOTHING(void) {\n";
    air_cpp_file << "}";

    air_cpp_file.close();
    std::cout << " - Air.cpp\n";

    // air.cpp gen end

    std::cout << " config generated files successfully.\n\n";

    return 0;
}
