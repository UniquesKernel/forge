#include "compile_commands.hpp"
#include "toml.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

static const std::string DIRECTORY             = std::filesystem::absolute(std::filesystem::current_path());
static const std::string OBJECT_FOLDER         = ".build/obj/";
static const std::string SRC_PATH              = "src";
static const std::string COMPILE_COMMANDS_FILE = "compile_commands.json";

std::vector<CommandJson> build_compile_commands_json(Settings settings) {
        std::vector<CommandJson> json_entries = std::vector<CommandJson>();

        for (const auto& entry : std::filesystem::directory_iterator(SRC_PATH)) {
                std::string filename   = entry.path().filename().string();
                std::string command    = settings.config.compiler + " " + entry.path().string() + settings.config.compiler_standard +
                                         " -g -Iinclude -c -o " + OBJECT_FOLDER +
                                         filename.substr(0, filename.find_last_of('.')) + ".o";
                CommandJson json_entry = {.directory = DIRECTORY, .command = command, .file = filename};
                json_entries.push_back(json_entry);
                printf("%s\n", json_entry.command.c_str());
                printf("%s\n", json_entry.file.c_str());

                printf("%s\n", json_entry.directory.c_str());
        }

        std::ofstream outfile(COMPILE_COMMANDS_FILE);
        system("mkdir -p .build/obj/");

        if (outfile.is_open()) {
                outfile << "[\n";
                bool is_first = true;
                for (const auto& json_entry : json_entries) {
                        if (!is_first) {
                                outfile << ",\n\t{\n";
                        } else {
                                outfile << "\t{\n";
                                is_first = false;
                        }
                        outfile << "\t\t\"directory\":" << "\"" << json_entry.directory << "\",\n";
                        outfile << "\t\t\"command\":" << "\"" << json_entry.command << "\",\n";
                        outfile << "\t\t\"file\":" << "\"" << json_entry.file << "\"\n";
                        outfile << "\t}";
                        printf("%s\n", std::filesystem::current_path().c_str());
                        printf("Executing the build command: %s\n", json_entry.command.c_str());
                }
                outfile << "\n]";
                outfile.close();
        }

        return json_entries;
}