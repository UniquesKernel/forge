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

const char* read_project_toml() {
        DEFER(safe_fclose) FILE* project_toml = fopen("forge.toml", "r");

        REQUIRE(project_toml != nullptr, nullptr);

        fseek(project_toml, 0, SEEK_END);
        const size_t PROJECT_TOML_SIZE = ftell(project_toml);
        fseek(project_toml, 0, SEEK_SET);

        REQUIRE(PROJECT_TOML_SIZE > 0, nullptr);

        char* src = (char*)malloc(PROJECT_TOML_SIZE + 1);

        REQUIRE(src != nullptr, nullptr);

        const size_t NBYTES_READ = fread(src, 1, PROJECT_TOML_SIZE, project_toml);

        INVARIANT(NBYTES_READ == PROJECT_TOML_SIZE);

        src[PROJECT_TOML_SIZE] = '\0';

        return (const char*)src;
}

int process_lib(TokenStream tokens, std::vector<Library>& libraries, int offset) {
        int     local_offset = offset + 1;
        Library current_lib  = {};

        while (local_offset < tokens.count) {
                Token& t = tokens.stream[local_offset];

                // 1. Stop if we hit a new section [[header]] or End of File
                if (t.type == TokenType::TOK_EOF || t.type == TokenType::TOK_LABEL) {
                        break;
                }

                // 2. Look for the "name" identifier
                if (t.type == TokenType::TOK_KEY && strcmp(t.lexeme, "name") == 0) {
                        int value_idx = local_offset + 1;

                        // Skip the '=' token if it exists (handles "name = " vs "name=")
                        if (value_idx < tokens.count && tokens.stream[value_idx].type == TokenType::TOK_EQUALS) {
                                value_idx++;
                        }

                        // 3. Safety check: ensure the value is actually there and is a string
                        if (value_idx < tokens.count && tokens.stream[value_idx].type == TokenType::TOK_VALUE) {
                                current_lib.lib_name = tokens.stream[value_idx].lexeme;
                                libraries.push_back(current_lib);

                                // Jump local_offset to the value we just consumed
                                local_offset = value_idx;
                        }
                }

                local_offset++;
        }

        return local_offset; // Return the position where the next parser should start
}

int process_config(TokenStream tokens, Config* config, int offset) {
        int i = offset + 1;

        while (i < tokens.count && tokens.stream[i].type != TokenType::TOK_EOF &&
               tokens.stream[i].type != TokenType::TOK_LABEL) {
                Token& key_tok = tokens.stream[i];

                // 1. Check for the Key
                if (key_tok.type == TokenType::TOK_KEY) {
                        int val_idx = i + 1;

                        // 2. Skip '=' if your lexer treats it as a separate token
                        if (val_idx < tokens.count && tokens.stream[val_idx].type == TokenType::TOK_EQUALS) {
                                val_idx++;
                        }

                        // 3. Safety check for the Value
                        if (val_idx < tokens.count) {
                                const char* val = tokens.stream[val_idx].lexeme;

                                if (strcmp(key_tok.lexeme, "name") == 0) {
                                        config->binary = val;
                                } else if (strcmp(key_tok.lexeme, "compiler") == 0) {
                                        config->compiler = "/usr/bin/" + std::string(val);
                                } else if (strcmp(key_tok.lexeme, "std") == 0) {
                                        config->compiler_standard = " -std=" + std::string(val);
                                }

                                // Jump i to the value we just consumed
                                i = val_idx;
                        }
                }
                i++;
        }
        return i;
}

int process_config(Token* tokens, Config* config, int offset) {
        int i = offset + 1;

        // Use TokenType::TOK_EOF and TOK_LABEL as boundaries
        while (tokens[i].type != TokenType::TOK_EOF && tokens[i].type != TokenType::TOK_LABEL) {
                // 1. Look for a KEY
                if (tokens[i].type == TokenType::TOK_KEY) {
                        const char* key_name = tokens[i].lexeme;
                        int         val_idx  = i + 1;

                        // 2. Skip the '=' token if your lexer generates it
                        if (tokens[val_idx].type == TokenType::TOK_EQUALS) {
                                val_idx++;
                        }

                        // 3. Ensure the value exists before accessing it
                        if (tokens[val_idx].type != TokenType::TOK_EOF) {
                                const char* val = tokens[val_idx].lexeme;

                                if (strcmp(key_name, "name") == 0) {
                                        config->binary = val;
                                } else if (strcmp(key_name, "compiler") == 0) {
                                        config->compiler = "/usr/bin/" + std::string(val);
                                } else if (strcmp(key_name, "std") == 0) {
                                        config->compiler_standard = " -std=" + std::string(val);
                                }

                                // Advance i to the value we just processed
                                i = val_idx;
                        }
                }
                i++;
        }

        // Return the new offset so the caller knows where we stopped
        return i;
}

Settings parse_project_file(TokenStream tokens) {
        Config               config = {};
        std::vector<Library> libraries;

        for (int i = 0; i < tokens.count;) { // Remove i++ from here to avoid double-stepping
                if (tokens.stream[i].type == TokenType::TOK_LABEL && strcmp(tokens.stream[i].lexeme, "config") == 0) {
                        i = process_config(tokens.stream, &config, i);
                } else if (tokens.stream[i].type == TokenType::TOK_LABEL &&
                           strcmp(tokens.stream[i].lexeme, "lib") == 0) {
                        i = process_lib(tokens, libraries, i);
                } else {
                        i++; // Just move forward if it's not a label we care about
                }
        }

        printf("config: { binary: %s, compiler: %s, standard: %s }\n", config.binary.c_str(), config.compiler.c_str(),
               config.compiler_standard.c_str());
        for (int i = 0; i < libraries.size(); i++) {
                printf("library: { lib name: %s }\n", libraries.at(i).lib_name.c_str());
        }
        return {.config = config, .libraries = libraries};
}
