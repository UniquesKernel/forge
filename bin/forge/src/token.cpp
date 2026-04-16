#include "token.hpp"
#include "assert.hpp"
#include "status.hpp"
#include "types.hpp"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>

// ========================================  CONSTANTS ========================================

static constexpr forge::u32 MAX_LEXEME_LENGTH = 100;

// ========================================  FORWARD DECLARED HELPER FUNCTIONS ========================================

/*!
  @brief Tokenize the next token from the TOML source

  @pre `tok` must not be `nullptr` — enforced by `REQUIRE(tok != nullptr, Error::NULL_PARAMETER)`
  @pre `tok->lexeme` must point to a heap-allocated buffer of at least `MAX_LEXEME_LENGTH` bytes — enforced by `REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER)`
  @pre `source` must not be `nullptr` — enforced by `REQUIRE(source != nullptr, Error::NULL_PARAMETER)`
  @pre `end` must point one past the last valid byte of the source
  @post `tok` is populated with the next token from the source; `TOK_EOF` if `*source >= end`, `TOK_UNKNOWN` for unrecognised characters
  @post `tok->lexeme` points to the heap-allocated buffer passed in via `tok`; the caller is responsible for freeing it
  @post `*source` is advanced past the consumed characters

  @param[out]    tok     the token to populate with the result
  @param[in,out] source  pointer to the current read position; advanced past the consumed token on return
  @param[in]     end     one-past-the-end pointer of the source buffer

  @return `Error::OK` on success; `Error::NULL_PARAMETER` if any pointer is `nullptr`; propagates any error returned by the underlying `process_*` or `skip_whitespace` helpers
*/
static Error                tokenize(Token* tok, const char** source, const char* end);

/*!
  @brief Advance the source pointer past any leading whitespace characters

  Skips spaces (`' '`), horizontal tabs (`'\t'`), carriage returns (`'\r'`), and newlines (`'\n'`).

  @pre `source` must not be `nullptr` — enforced by `REQUIRE(source != nullptr, Error::NULL_PARAMETER)`
  @pre `*source` must not be `nullptr` — enforced by `REQUIRE(*source != nullptr, Error::NULL_PARAMETER)`
  @post `*source` points to the first non-whitespace character, or remains unchanged if none is present
  @post `*source` does not exceed `end` — guaranteed by the `end >= src` loop guard

  @param[in,out] source  pointer to the current read position; advanced past all leading whitespace on return
  @param[in]     end     one-past-the-end pointer of the source buffer

  @return `Error::OK` on success; `Error::NULL_PARAMETER` if `source` or `*source` is `nullptr`
*/
static Error                skip_whitespace(const char** source, const char* end);

/*!
  @brief Consume and build a `TOK_KEY` token from the source

  Reads consecutive alphanumeric characters, underscores (`'_'`), and hyphens (`'-'`) into `tok->lexeme`.

  @pre `tok` must not be `nullptr` — enforced by `REQUIRE(tok != nullptr, Error::NULL_PARAMETER)`
  @pre `tok->lexeme` must not be `nullptr` — enforced by `REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER)`
  @pre `source` must not be `nullptr` — enforced by `REQUIRE(source != nullptr, Error::NULL_PARAMETER)`
  @pre `**source` must satisfy `isalnum`, `'_'`, or `'-'` — enforced by `REQUIRE(..., Error::INVALID_ARGUMENT)`
  @post `tok->type` is set to `TOK_KEY`
  @post `tok->lexeme` contains the null-terminated key string
  @post `*source` is advanced past the consumed characters
  @post `*source` does not exceed `end` — guaranteed by the `end >= src` loop guard

  @param[out]    tok     the token to populate
  @param[in,out] source  pointer to the current read position; advanced past the key on return
  @param[in]     end     one-past-the-end pointer of the source buffer

  @return `Error::OK` on success; `Error::NULL_PARAMETER` if any pointer is `nullptr`; `Error::INVALID_ARGUMENT` if `**source` is not a valid key-start character
*/
static Error                process_key(Token* tok, const char** source, const char* end);

/*!
  @brief Consume and build a `TOK_VALUE` token from a double-quoted string in the source

  Reads characters between the opening and closing `'"'` delimiters (up to 50 characters) into `tok->lexeme`.
  Both delimiter characters are consumed and not included in the lexeme.

  @pre `tok` must not be `nullptr` — enforced by `REQUIRE(tok != nullptr, Error::NULL_PARAMETER)`
  @pre `tok->lexeme` must not be `nullptr` — enforced by `REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER)`
  @pre `source` must not be `nullptr` — enforced by `REQUIRE(source != nullptr, Error::NULL_PARAMETER)`
  @pre `**source` must be `'"'` — enforced by `REQUIRE(**source == '"', Error::INVALID_ARGUMENT)`
  @post `tok->type` is set to `TOK_VALUE`
  @post `tok->lexeme` contains the null-terminated string value, without surrounding quotes
  @post `*source` is advanced past the closing `'"'`
  @post `*source` (before the closing quote skip) does not exceed `end` — guaranteed by the `end >= src` loop guard
  @post if `src >= end` after the loop (unterminated string), the function returns before the closing quote skip — enforced by `REQUIRE(src < end, Error::UNEXPECTED_EOF)`

  @param[out]    tok     the token to populate
  @param[in,out] source  pointer to the current read position; advanced past the closing quote on return
  @param[in]     end     one-past-the-end pointer of the source buffer

  @return `Error::OK` on success; `Error::NULL_PARAMETER` if any pointer is `nullptr`; `Error::INVALID_ARGUMENT` if `**source` is not `'"'`; `Error::UNEXPECTED_EOF` if the string is unterminated
*/
static Error                process_value(Token* tok, const char** source, const char* end);

/*!
  @brief Consume and build a `TOK_INTEGER` token from a sequence of digit characters in the source

  @pre `tok` must not be `nullptr` — enforced by `REQUIRE(tok != nullptr, Error::NULL_PARAMETER)`
  @pre `tok->lexeme` must not be `nullptr` — enforced by `REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER)`
  @pre `source` must not be `nullptr` — enforced by `REQUIRE(source != nullptr, Error::NULL_PARAMETER)`
  @pre `**source` must satisfy `isdigit` — enforced by `REQUIRE(isdigit(**source), Error::INVALID_ARGUMENT)`
  @post `tok->type` is set to `TOK_INTEGER`
  @post `tok->lexeme` contains the null-terminated decimal digit string
  @post `*source` is advanced past the consumed digits
  @post `*source` does not exceed `end` — guaranteed by the `end >= src` loop guard

  @param[out]    tok     the token to populate
  @param[in,out] source  pointer to the current read position; advanced past the digits on return
  @param[in]     end     one-past-the-end pointer of the source buffer

  @return `Error::OK` on success; `Error::NULL_PARAMETER` if any pointer is `nullptr`; `Error::INVALID_ARGUMENT` if `**source` is not a digit
*/
static Error                process_digit(Token* tok, const char** source, const char* end);

/*!
  @brief Consume and build a `TOK_LABEL` token from a double-bracket section header in the source

  Reads the label name between the `[[` and `]]` delimiters into `tok->lexeme`.
  All four bracket characters are consumed and not included in the lexeme.

  @pre `tok` must not be `nullptr` — enforced by `REQUIRE(tok != nullptr, Error::NULL_PARAMETER)`
  @pre `tok->lexeme` must not be `nullptr` — enforced by `REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER)`
  @pre `source` must not be `nullptr` — enforced by `REQUIRE(source != nullptr, Error::NULL_PARAMETER)`
  @pre `**source` must be `'['` and `*(*source + 1)` must also be `'['` — enforced by `REQUIRE(..., Error::INVALID_ARGUMENT)`
  @post `tok->type` is set to `TOK_LABEL`
  @post `tok->lexeme` contains the null-terminated label name, without surrounding brackets
  @post `*source` is advanced past the closing `]]`
  @post `*source` (before the closing `]]` skip) does not exceed `end` — guaranteed by the `end >= src` loop guard
  @post if `src >= end` after the loop (unterminated label), the function returns before either closing bracket skip — enforced by `REQUIRE(src < end, Error::UNEXPECTED_EOF)`

  @param[out]    tok     the token to populate
  @param[in,out] source  pointer to the current read position; advanced past the closing `]]` on return
  @param[in]     end     one-past-the-end pointer of the source buffer

  @return `Error::OK` on success; `Error::NULL_PARAMETER` if any pointer is `nullptr`; `Error::INVALID_ARGUMENT` if `**source` is not `'[['`; `Error::UNEXPECTED_EOF` if the label is unterminated
*/
static Error                process_label(Token* tok, const char** source, const char* end);

// ========================================  IMPLEMENTATION ========================================

namespace forge::token {
Error lexing(forge::toml::TomlSource toml_src, TokenStream* stream) {
        REQUIRE(stream != nullptr, Error::NULL_PARAMETER);
        REQUIRE(stream->stream == nullptr, Error::INVALID_ARGUMENT);
        REQUIRE(toml_src.source != nullptr, Error::INVALID_ARGUMENT);
        REQUIRE(toml_src.size > 0, Error::INVALID_ARGUMENT);

        Error       err          = Error::OK;
        Token*      token_stream = (Token*)malloc(sizeof(*token_stream) * toml_src.size);
        forge::i32  token_count  = 0;
        const char* src          = toml_src.source;
        const char* end          = src + toml_src.size;
        off_t       i            = 0;

        // NOTE:(UniquesKernel) toml_src.size is a tight upper bound on loop iterations:
        // it prevents infinite loops without risking an early exit, since a source of N
        // characters can produce at most N tokens.
        while (src < end && i < toml_src.size) {
                Token tok  = {};
                tok.lexeme = (char*)malloc(MAX_LEXEME_LENGTH);

                if (err = tokenize(&tok, &src, end); err != Error::OK) {
                        free(tok.lexeme);
                        goto cleanup;
                }

                token_stream[token_count++] = tok;

                if (tok.type == TokenType::TOK_EOF) {
                        break;
                }
        }

        stream->stream = token_stream;
        stream->count  = token_count;

        return err;

cleanup:
        for (forge::i32 j = 0; j < token_count; j++) {
                free(token_stream[j].lexeme);
        }
        free(token_stream);
        return err;
}
} // namespace forge::token

// ======================================== HELPER FUNCTIONS ========================================

static Error tokenize(Token* tok, const char** source, const char* const end) {
        REQUIRE(tok != nullptr, Error::NULL_PARAMETER);
        REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER);
        REQUIRE(source != nullptr, Error::NULL_PARAMETER);

        const char* src = *source;

        if (src >= end) {
                tok->lexeme[0] = 'E';
                tok->type      = TokenType::TOK_EOF;
                goto return_label;
        }

        if (Error err = skip_whitespace(&src, end); err != Error::OK) {
                return err;
        }

        if (src >= end) {
                tok->lexeme[0] = 'E';
                tok->type      = TokenType::TOK_EOF;
                goto return_label;
        }

        if (isdigit(*src)) {
                if (Error err = process_digit(tok, &src, end); err != Error::OK) {
                }
                REQUIRE(process_digit(tok, &src, end) == Error::OK, Error::INVALID_ARGUMENT);
                goto return_label;
        }

        if (isalnum(*src) || *src == '_' || *src == '-') {
                REQUIRE(process_key(tok, &src, end) == Error::OK, Error::INVALID_ARGUMENT);
                goto return_label;
        }

        if (*src == '[') {
                REQUIRE(process_label(tok, &src, end) == Error::OK, Error::INVALID_ARGUMENT);
                goto return_label;
        }

        if (*src == '=') {
                tok->type      = TokenType::TOK_EQUALS;
                tok->lexeme[0] = *(src++);
                tok->lexeme[1] = '\0';
                goto return_label;
        }

        if (*src == '"') {
                REQUIRE(process_value(tok, &src, end) == Error::OK, Error::INVALID_ARGUMENT);
                goto return_label;
        }

        tok->lexeme[0] = 'U';
        tok->type      = TokenType::TOK_UNKNOWN;
        src++;

return_label:
        *source = src;
        return Error::OK;
}

static Error skip_whitespace(const char** source, const char* end) {
        REQUIRE(source != nullptr, Error::NULL_PARAMETER);
        REQUIRE(*source != nullptr, Error::NULL_PARAMETER);

        const char* src = *source;

        forge::u32  i   = 0;
        while (src < end && (i++ < (MAX_LEXEME_LENGTH - 1) &&
                             ((*src) == ' ' || (*src) == '\t' || (*src) == '\n' || (*src) == '\r'))) {
                src++;
        }

        *source = src;
        return Error::OK;
}

static Error process_key(Token* tok, const char** source, const char* end) {
        REQUIRE(tok != nullptr, Error::NULL_PARAMETER);
        REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER);
        REQUIRE(source != nullptr, Error::NULL_PARAMETER);
        REQUIRE(isalnum(**source) || **source == '_' || **source == '-', Error::INVALID_ARGUMENT);

        const char* src = *source;

        forge::u32  i   = 0;
        while (src < end && i < (MAX_LEXEME_LENGTH - 1) && (isalnum(*src) || (*src) == '_' || (*src) == '-')) {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type        = TokenType::TOK_KEY;

        *source          = src;
        return Error::OK;
}

static Error process_value(Token* tok, const char** source, const char* end) {
        REQUIRE(tok != nullptr, Error::NULL_PARAMETER);
        REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER);
        REQUIRE(source != nullptr, Error::NULL_PARAMETER);
        REQUIRE(**source == '"', Error::INVALID_ARGUMENT);

        const char* src = *source;

        // NOTE: Skip starting '\"' character
        src++;

        forge::u32 i = 0;
        while (src < end && (*src && *src != '"') && i < (MAX_LEXEME_LENGTH - 1)) {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type        = TokenType::TOK_VALUE;

        // NOTE: Skip the closing '\"' character
        REQUIRE(src < end, Error::UNEXPECTED_EOF);
        src++;

        *source = src;
        return Error::OK;
}

static Error process_digit(Token* tok, const char** source, const char* end) {
        REQUIRE(tok != nullptr, Error::NULL_PARAMETER);
        REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER);
        REQUIRE(source != nullptr, Error::NULL_PARAMETER);
        REQUIRE(isdigit(**source), Error::INVALID_ARGUMENT);

        const char* src = *source;

        forge::u32  i   = 0;
        while (src < end && isdigit(*src) && (i < MAX_LEXEME_LENGTH - 1)) {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type        = TokenType::TOK_INTEGER;

        *source          = src;
        return Error::OK;
}

static Error process_label(Token* tok, const char** source, const char* end) {
        REQUIRE(tok != nullptr, Error::NULL_PARAMETER);
        REQUIRE(tok->lexeme != nullptr, Error::NULL_PARAMETER);
        REQUIRE(source != nullptr, Error::NULL_PARAMETER);

        const char* src = *source;

        REQUIRE(src < end, Error::UNEXPECTED_EOF);
        src++; // skip first '[' character
        REQUIRE(src < end, Error::UNEXPECTED_EOF);
        src++; // skip second '[' character

        forge::u32 i = 0;
        while (src < end && *src != ']' && (i < MAX_LEXEME_LENGTH - 1)) {
                tok->lexeme[i++] = *(src++);
        }
        tok->lexeme[i++] = '\0';
        tok->type        = TokenType::TOK_LABEL;

        // NOTE: Skip the closing ']]' characters
        REQUIRE(src < end, Error::UNEXPECTED_EOF);
        src++;
        REQUIRE(src < end, Error::UNEXPECTED_EOF);
        src++;

        *source = src;
        return Error::OK;
}
