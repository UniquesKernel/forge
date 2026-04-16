#include "parser.hpp"
#include "assert.hpp"
#include "token.hpp"
#include "types.hpp"

// ======================================== FORWARD DECLARED HELPER FUNCTIONS ========================================

/*!
  @brief Processes all key-value pairs that fall under a library label in the token stream

  @pre `tokens` must not be a valid `TokenStream`
  @pre `offset` must not be `nullptr`

  @post `libraries` has been populated with an additional library that should be compiled
  @post `*offset` is advanced past the last token consumed

  @param[in]     tokens  the token stream to parse
  @param[out]    libraries  the library array to populate with the libraries that should be compiled
  @param[in,out] offset  the current position in the token stream; advanced past the last consumed token on return

  @retval Error::OK               Parsing completed successfully
  @retval Error::INVALID_ARGUMENT One or more arguments are invalid
  @retval Error::NULL_PARAMETER   One or more required pointers are null
  @retval Error::UNEXPECTED_EOF   The token stream ended before a key-value pair was complete
*/
static Error process_lib(TokenStream tokens, std::vector<forge::parser::Library>& libraries, forge::u32* offset);

/*!
  @brief Processes all key-value pairs that fall under a binary label in the token stream

  @pre `tokens` must not be a valid `TokenStream`
  @pre `offset` must not be `nullptr`

  @post `binaries` has been populated with an additional binary that should be compiled
  @post `*offset` is advanced past the last token consumed

  @param[in]     tokens  the token stream to parse
  @param[out]    binaries  the binary array to populate with the binaries that should be compiled
  @param[in,out] offset  the current position in the token stream; advanced past the last consumed token on return

  @retval Error::OK               Parsing completed successfully
  @retval Error::INVALID_ARGUMENT One or more arguments are invalid
  @retval Error::NULL_PARAMETER   One or more required pointers are null
  @retval Error::UNEXPECTED_EOF   The token stream ended before a key-value pair was complete
*/
static Error process_bin(TokenStream tokens, std::vector<forge::parser::Binary>& binaries, forge::u32* offset);

/*!
  @brief Processes all key-value pairs that fall under a config label in the token stream

  @pre `tokens` must not be a valid `TokenStream`
  @pre `config` must not be `nullptr`
  @pre `offset` must not be `nullptr`

  @post `config` is populated with the key-value pairs found under the config label
  @post `*offset` is advanced past the last token consumed

  @param[in]     tokens  the token stream to parse
  @param[out]    config  the config structure to populate with the parsed key-value pairs
  @param[in,out] offset  the current position in the token stream; advanced past the last consumed token on return

  @retval Error::OK               Parsing completed successfully
  @retval Error::INVALID_ARGUMENT One or more arguments are invalid
  @retval Error::NULL_PARAMETER   One or more required pointers are null
  @retval Error::UNEXPECTED_EOF   The token stream ended before a key-value pair was complete
*/
static Error process_config(TokenStream tokens, forge::parser::Config* config, forge::u32* offset);

// ======================================== IMPLEMENTATION ========================================

namespace forge::parser {
Settings parse_token_stream(TokenStream tokens) {
        printf("HERE?\n");
        Config               config = {};
        std::vector<Library> libraries;
        std::vector<Binary>  binaries;

        // NOTE:(UniquesKernel) `i` is purposefully not incremented in the loop header
        // This is done to prevent double stepping during parsing
        for (forge::u32 i = 0; i < tokens.count;) {
                if (tokens.stream[i].type == TokenType::TOK_LABEL && strcmp(tokens.stream[i].lexeme, "config") == 0) {
                        process_config(tokens, &config, &i);
                } else if (tokens.stream[i].type == TokenType::TOK_LABEL &&
                           strcmp(tokens.stream[i].lexeme, "lib") == 0) {
                        process_lib(tokens, libraries, &i);
                } else if (tokens.stream[i].type == TokenType::TOK_LABEL &&
                           strcmp(tokens.stream[i].lexeme, "bin") == 0) {
                        process_bin(tokens, binaries, &i);
                } else {
                        i++; // Just move forward if it's not a label we care about
                }
        }
        printf("HERE?? size of lib: %lu, size of bin %lu\n", libraries.size(), binaries.size());
        return (Settings){.config = config, .libraries = libraries, .binaries = binaries};
}
} // namespace forge::parser

// ======================================== HELPER FUNCTIONS ========================================

static Error process_lib(TokenStream tokens, std::vector<forge::parser::Library>& libraries, forge::u32* offset) {
        REQUIRE(tokens.stream != nullptr, Error::INVALID_ARGUMENT);
        REQUIRE(tokens.count > 0, Error::INVALID_ARGUMENT);
        REQUIRE(offset != nullptr, Error::NULL_PARAMETER);

        forge::parser::Library lib = {};
        forge::u32             i   = (*offset) + 1;

        while (i < tokens.count && tokens.stream[i].type != TokenType::TOK_EOF &&
               tokens.stream[i].type != TokenType::TOK_LABEL) {
                Token tok = tokens.stream[i];

                if (tok.type == TokenType::TOK_EQUALS) {
                        // clang-format off
                        // NOTE:(UniquesKernel) The following reasoning justifies the use of an invariant:
                        // 1. `typeof(*offset) == u32`
                        // 2. `0 <= *offset`
                        // 3. `0 <= *offset + 1 - 1`; substituting `i = *offset + 1`
                        // 4. `0 <= i - 1`
                        // 5. since `i` only increments `0 <= i - 1` always remains true, and we use an invariant to ensure it
                        // clang-format on
                        INVARIANT((i - 1) < tokens.count);
                        Token left = tokens.stream[i - 1];
                        REQUIRE((i + 1) < tokens.count, Error::UNEXPECTED_EOF);
                        Token right = tokens.stream[i + 1];

                        if (left.type == TokenType::TOK_KEY && right.type == TokenType::TOK_VALUE) {
                                if (strcmp(left.lexeme, "name") == 0) {
                                        lib.lib_name = right.lexeme;
                                        libraries.push_back(lib);
                                }
                                i++;
                        }
                }
                i++;
        }

        *offset = i;
        return Error::OK;
}

static Error process_bin(TokenStream tokens, std::vector<forge::parser::Binary>& binaries, forge::u32* offset) {
        REQUIRE(tokens.stream != nullptr, Error::INVALID_ARGUMENT);
        REQUIRE(tokens.count > 0, Error::INVALID_ARGUMENT);
        REQUIRE(offset != nullptr, Error::NULL_PARAMETER);

        forge::parser::Binary bin = {};
        forge::u32            i   = (*offset) + 1;

        while (i < tokens.count && tokens.stream[i].type != TokenType::TOK_EOF &&
               tokens.stream[i].type != TokenType::TOK_LABEL) {
                Token tok = tokens.stream[i];

                if (tok.type == TokenType::TOK_EQUALS) {
                        // clang-format off
                        // NOTE:(UniquesKernel) The following reasoning justifies the use of an invariant:
                        // 1. `typeof(*offset) == u32`
                        // 2. `0 <= *offset`
                        // 3. `0 <= *offset + 1 - 1`; substituting `i = *offset + 1`
                        // 4. `0 <= i - 1`
                        // 5. since `i` only increments `0 <= i - 1` always remains true, and we use an invariant to ensure it
                        // clang-format on
                        INVARIANT((i - 1) < tokens.count);
                        Token left = tokens.stream[i - 1];
                        REQUIRE((i + 1) < tokens.count, Error::UNEXPECTED_EOF);
                        Token right = tokens.stream[i + 1];

                        if (left.type == TokenType::TOK_KEY && right.type == TokenType::TOK_VALUE) {
                                if (strcmp(left.lexeme, "name") == 0) {
                                        bin.bin_name = right.lexeme;
                                        binaries.push_back(bin);
                                }
                                i++;
                        }
                }
                i++;
        }

        *offset = i;
        return Error::OK;
}

static Error process_config(TokenStream tokens, forge::parser::Config* config, forge::u32* offset) {
        REQUIRE(tokens.stream != nullptr, Error::INVALID_ARGUMENT);
        REQUIRE(tokens.count > 0, Error::INVALID_ARGUMENT);
        REQUIRE(config != nullptr, Error::NULL_PARAMETER);
        REQUIRE(offset != nullptr, Error::NULL_PARAMETER);

        forge::u32 i = (*offset) + 1;

        while (i < tokens.count && tokens.stream[i].type != TokenType::TOK_EOF &&
               tokens.stream[i].type != TokenType::TOK_LABEL) {
                if (tokens.stream[i].type == TokenType::TOK_EQUALS) {
                        // clang-format off
                        // NOTE:(UniquesKernel) The following reasoning justifies the use of an invariant:
                        // 1. `typeof(*offset) == u32`
                        // 2. `0 <= *offset`
                        // 3. `0 <= *offset + 1 - 1`; substituting `i = *offset + 1`
                        // 4. `0 <= i - 1`
                        // 5. since `i` only increments `0 <= i - 1` always remains true, and we use an invariant to ensure it
                        // clang-format on
                        INVARIANT((i - 1) < tokens.count);
                        Token left = tokens.stream[i - 1];
                        REQUIRE((i + 1) < tokens.count, Error::UNEXPECTED_EOF);
                        Token right = tokens.stream[i + 1];

                        if (left.type == TokenType::TOK_KEY && right.type == TokenType::TOK_VALUE) {
                                const char* key_name = left.lexeme;
                                const char* val      = right.lexeme;

                                if (strcmp(key_name, "name") == 0) {
                                        config->binary = val;
                                } else if (strcmp(key_name, "compiler") == 0) {
                                        config->compiler = "/usr/bin/" + std::string(val);
                                } else if (strcmp(key_name, "std") == 0) {
                                        config->compiler_standard = "-std=" + std::string(val);
                                }

                                i++;
                        }
                }
                i++;
        }

        *offset = i;
        return Error::OK;
}
