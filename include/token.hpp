#ifndef FORGE_TOKEN_HPP
#define FORGE_TOKEN_HPP

#include "toml.hpp"

enum class TokenType : int {
        TOK_KEY     = 1,
        TOK_EQUALS  = 2,
        TOK_VALUE   = 3,
        TOK_INTEGER = 4,
        TOK_LABEL   = 5,
        TOK_EOF     = 6,
        TOK_UNKNOWN = 7
};

struct Token {
        TokenType type   = TokenType::TOK_UNKNOWN;
        char*     lexeme = nullptr;
};

typedef struct TokenStream {
        Token*     stream = nullptr;
        forge::i32 count  = 0;
} TokenStream;

namespace forge::token {

/*!
  @brief Lex an entire TOML source into a token stream

  @pre `src.source` must point to a valid read-only memory region and `src.size > 0`
  @pre `token_stream` must not be null
  @post On `OK`: `token_stream->stream` points to a heap-allocated array of `token_stream->count` tokens; the caller is responsible for freeing it
  @post On error: the contents of `*token_stream` are unspecified and must not be read

  @param[in]  src           the TOML source to lex, as produced by `forge::toml::map_config`
  @param[out] token_stream  receives the resulting token stream on success

  @return Error enumeration where `OK` indicates success and all other values indicate an error

  @note The function does not throw - but can abort if internal invariants are violated
*/
[[nodiscard("A Token Stream should not be ignored, "
            "as this will lead to a memory leak")]]
Error lexing(forge::toml::TomlSource src, TokenStream* token_stream);
} // namespace forge::token

#endif
