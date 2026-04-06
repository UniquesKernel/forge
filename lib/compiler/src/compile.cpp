#include "compile.hpp"
#include "constants.hpp"
#include "toml.hpp"
#include <filesystem>
#include <vector>

void compile(std::vector<CommandJson> commandsJson) {
        for (const auto& entry : commandsJson) {
                system(entry.command.c_str());
        }
}

int link(Settings settings) {
        using namespace std::filesystem;

        std::string cmd = settings.config.compiler + settings.config.compiler_standard + " -g -O0 ";
        for (const directory_entry& entry : recursive_directory_iterator(OBJECT_FOLDER)) {
                if (entry.is_directory()) {
                        continue;
                }
                cmd += entry.path().string() + " ";
        }
        cmd                           += " -o " + BINRARY_FOLDER + settings.config.binary;

        bool is_binary_folder_created  = create_directory(BINRARY_FOLDER);
        printf("Executing: %s\n", cmd.c_str());
        int result = system(cmd.c_str());

        if (is_binary_folder_created) {
                printf("Succesfully Created .build/bin\n");
        } else {
                printf("Failed to construct .build/bin\n");
        }

        if (result >= 0) {
                printf("Succesfully build, %s\n", settings.config.binary.c_str());
        } else {
                printf("Failed to build: %s\n", settings.config.binary.c_str());
        }

        return 0;
}
