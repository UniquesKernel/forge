#include "token.hpp"
#include <cctype>
#include <cstdlib>

namespace forge::token {
void skip_whitespace(char** source) {
        char* src = *source;

        while ((*src) == ' ' || (*src) == '\t') {
                src++;
        }

        *source = src;
}

Token process_key(char** source) {
        char* src = *source;

        Token tok = {};

        int   i   = 0;
        while (isalnum(*src) || (*src) == '_' || (*src) == '-') {
                tok.lexeme[i++] = *(src++);
        }
        tok.type = TokenType::TOK_KEY;

        *source  = src;
        return tok;
}

Token process_value(char** source) {
        char* src = *source;
        Token tok = {};

        // NOTE: Skip starting '\"' character
        src++;

        int       i         = 0;
        const int token_max = 50;
        while (*src && *src != '"' && i < token_max) {
                tok.lexeme[i++] = *src++;
        }
        tok.type = TokenType::TOK_VALUE;

        // NOTE: Skip the closing '\"' character
        src++;

        *source = src;
        return tok;
}

Token process_digit(char** source) {
        char* src = *source;
        Token tok = {};
        int   i   = 0;
        
        while (isdigit(*src)) {
                tok.lexeme[i++] = *src++;
        }
        tok.type = TokenType::TOK_INTEGER;
        
        *source  = src;
        return tok;
}

Token tokenize(char** source) {
        Token tok = {};

        char* src = *source;

        if (*src == '\0') {
                tok.type = TokenType::TOK_EOF;
                goto return_label;
        }

        skip_whitespace(&src);

        if (isalnum(*src) || *src == '_' || *src == '-') {
                tok = process_key(&src);
                goto return_label;
        }

        if (*src == '=') {
                tok.type      = TokenType::TOK_EQUALS;
                tok.lexeme[0] = *src++;
                goto return_label;
        }

        if (*src == '"') {
                tok = process_value(&src);
                goto return_label;
        }

        if (isdigit(*src)) {
                tok = process_digit(&src);
                goto return_label;
        }

        tok.type = TokenType::TOK_UNKNOWN;
        src++;

    return_label:
        *source = src;
        return tok;
}

TokenStream lexing(char* src) {
        Token* token_steam = (Token*)malloc(sizeof(Token) * 100);

        int    token_count = 0;
        Token  tok         = tokenize(&src);
        while (tok.type != TokenType::TOK_EOF && token_count < 100) {
                token_steam[token_count++] = tok;
                tok                        = tokenize(&src);
        }

        return {.stream = token_steam, .count = token_count};
}
} // namespace forge::token
