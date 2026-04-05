#include "toml.hpp"
#include "assert.hpp"
#include "token.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
void safe_fclose(FILE** ptr) {
        if (ptr == NULL || *ptr == NULL) {
                return;
        }
        fclose(*ptr);
}
} // namespace

char* read_project_toml() {
        DEFER(safe_fclose) FILE* project_toml = fopen("forge.toml", "r");

        REQUIRE(project_toml != nullptr, nullptr);

        fseek(project_toml, 0, SEEK_END);
        size_t length = ftell(project_toml);
        fseek(project_toml, 0, SEEK_SET);

        REQUIRE(length > 0, nullptr);

        char* src = (char*)malloc(length + 1);

        REQUIRE(src != nullptr, nullptr);

        size_t read_length = fread(src, 1, length, project_toml);

        INVARIANT(read_length == length);

        src[length] = '\0';

        return src;
}

int process_lib(TokenStream tokens, std::vector<Library>& libraries, int offset) {
        int local_offset = offset + 1;
        int parse_count = 0;
        Library library = {};
        while (tokens.stream[local_offset].type != TokenType::TOK_EOF && tokens.stream[local_offset].type != TokenType::TOK_LABEL) {
                if (strcmp(tokens.stream[local_offset].lexeme, "name") == 0) {
                        library.lib_name = tokens.stream[local_offset + 2].lexeme;
                        libraries.push_back(library);
                }
                local_offset += 3;
                parse_count++;
        }
        
        return parse_count;
}

int process_config(Token* tokens, Config* config, int offset) {
        int local_offset = offset + 1;
        int parse_count = 0;
        while (tokens[local_offset].type != TokenType::TOK_EOF && tokens[local_offset].type != TokenType::TOK_LABEL) {
                if (strcmp(tokens[local_offset].lexeme, "name") == 0) {
                        config->binary = tokens[local_offset + 2].lexeme;
                }

                if (strcmp(tokens[local_offset].lexeme, "compiler") == 0) {
                        config->compiler = "/usr/bin/" + std::string(tokens[local_offset + 2].lexeme);
                }

                if (strcmp(tokens[local_offset].lexeme, "std") == 0) {
                        config->compiler_standard = " -std=" + std::string(tokens[local_offset + 2].lexeme);
                }
                local_offset += 3;
                parse_count++;
        }
        return parse_count;
}

Settings parse_project_file(TokenStream tokens) {
        std::string binary_name       = "";
        std::string compiler_name     = "/usr/bin/";
        std::string compiler_standard = " -std=";

        Config config = {};
        std::vector<Library> libraries = std::vector<Library>();
        for (int i = 0; i < tokens.count; i++) {
                if (tokens.stream[i].type == TokenType::TOK_LABEL && strcmp(tokens.stream[i].lexeme, "config") == 0) {   
                        int parse_length = process_config(tokens.stream, &config, i);
                        i += (3 * parse_length);
                }

                if (tokens.stream[i].type == TokenType::TOK_LABEL && strcmp(tokens.stream[i].lexeme, "lib") == 0) {   
                        int parse_length = process_lib(tokens, libraries, i);
                        i += (3 * parse_length);
                }
        }

        printf("config: { binary: %s, compiler: %s, standard: %s }\n", config.binary.c_str(), config.compiler.c_str(), config.compiler_standard.c_str());
        for (int i = 0; i < libraries.size(); i++) {
                printf("library: { lib name: %s }\n", libraries.at(i).lib_name.c_str());
        }
        return {.config = config, .libraries = libraries};
}
