#include "compile_commands.hpp"
#include "constants.hpp"
#include "toml.hpp"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

std::vector<CommandJson> build_compile_commands_json(Settings* settings, const std::filesystem::path path) {
        std::vector<CommandJson> json_entries = std::vector<CommandJson>();

        std::filesystem::create_directories(OBJECT_FOLDER);

        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_directory() || entry.path().extension() != SOURCE_FILE_EXTENSION) {
                        continue;
                }

                const std::filesystem::path source_path = entry.path();
                const std::string           filename    = source_path.filename().string();
                const std::filesystem::path obj_dir  = std::filesystem::path(OBJECT_FOLDER) / source_path.parent_path();
                std::filesystem::path       obj_file = obj_dir / source_path.stem();
                obj_file += OBJECT_FILE_EXTENSION;

                std::filesystem::create_directories(obj_dir);

                std::string command = std::string(settings->config.compiler)
                                          .append(" ")
                                          .append(source_path.string())
                                          .append(" ")
                                          .append(settings->config.compiler_standard)
                                          .append(" -g -Iinclude -c -o ")
                                          .append(obj_file.string());

                CommandJson json_entry = {.directory = DIRECTORY, .command = command, .file = filename};
                json_entries.push_back(json_entry);
        }

        for (const auto& json : json_entries) {
                printf("COMMANDS: %s\n", json.command.c_str());
        }
        std::ofstream outfile(COMPILE_COMMANDS_FILE);

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
