#include "toml.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
void safe_fclose(FILE** ptr) {
        if (ptr == NULL || *ptr == NULL) {
                return;
        }
        fclose(*ptr);
}
} // namespace

char* read_project_file() {
        __attribute__((cleanup(safe_fclose))) FILE* project_toml = fopen("forge.toml", "r");

        if (!project_toml) {
                return nullptr;
        }

        fseek(project_toml, 0, SEEK_END);
        long length = ftell(project_toml);
        fseek(project_toml, 0, SEEK_SET);

        if (length < 0) {
                return nullptr;
        }

        char*  src         = (char*)malloc(length + 1);

        size_t read_length = fread(src, 1, length, project_toml);

        if (read_length != (size_t)length) {
                return nullptr;
        }

        src[length] = '\0';

        return src;
}

Settings parse_project_file(TokenStream tokens) {
        std::string binary_name = "";
        std::string compiler_name = "";
        for (int i = 0; i < tokens.count; i++) {
                if (strcmp(tokens.stream[i].lexeme, "name") == 0 && i + 2 < tokens.count) {
                        binary_name = tokens.stream[i + 2].lexeme;
                }
                if (strcmp(tokens.stream[i].lexeme, "compiler") == 0 && i + 2 < tokens.count) {
                        printf("token_stream: %s\n", tokens.stream[i + 2].lexeme);
                        compiler_name = tokens.stream[i + 2].lexeme;
                        compiler_name = "/usr/bin/" + compiler_name;
                }
        }

        return {.binary_name = binary_name, .compiler = compiler_name};
}