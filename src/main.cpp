#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

enum class TokenType : int {
  TOK_KEY = 1,
  TOK_EQUALS = 2,
  TOK_STRING = 3,
  TOK_INTEGER = 4,
  TOK_EOF = 5,
  TOK_UNKNOWN = 6
};

struct Token {
  TokenType type;
  char lexeme[64];
};

char *src;

void skip_whitespace() {
  while (*src == ' ' || *src == '\t')
    src++;
}

Token tokenize() {
  Token tok = {};
  skip_whitespace();

  if (*src == '\0') {
    tok.type = TokenType::TOK_EOF;
    return tok;
  }

  // Parse key (alphanumeric, _, -)
  if (isalnum(*src) || *src == '_' || *src == '-') {
    int i = 0;
    while (isalnum(*src) || *src == '_' || *src == '-') {
      tok.lexeme[i++] = *src++;
    }
    tok.type = TokenType::TOK_KEY;
    return tok;
  }

  // Parse '='
  if (*src == '=') {
    tok.type = TokenType::TOK_EQUALS;
    tok.lexeme[0] = *src++;
    return tok;
  }

  // Parse string value
  if (*src == '"') {
    src++; // skip "
    int i = 0;
    while (*src && *src != '"') {
      tok.lexeme[i++] = *src++;
    }
    src++; // skip closing "
    tok.type = TokenType::TOK_STRING;
    return tok;
  }

  // Parse integer
  if (isdigit(*src)) {
    int i = 0;
    while (isdigit(*src)) {
      tok.lexeme[i++] = *src++;
    }
    tok.type = TokenType::TOK_INTEGER;
    return tok;
  }

  tok.type = TokenType::TOK_UNKNOWN;
  src++;
  return tok;
}

void safe_fclose(FILE **ptr) {
  if (ptr == NULL || *ptr == NULL) {
    return;
  }
  fclose(*ptr);
}

void safe_free(void *ptr) {
  if ((void **)ptr == NULL || *((void **)(ptr)) == NULL) {
    return;
  }
  free(*((void **)ptr));
}

Token *token_steam;
int main(void) {
  printf("This is running forge v0.1");
  __attribute__((cleanup(safe_fclose))) FILE *project_toml =
      fopen("project.toml", "r");

  if (!project_toml) {
    return EXIT_FAILURE;
  }

  fseek(project_toml, 0, SEEK_END);
  long length = ftell(project_toml);
  fseek(project_toml, 0, SEEK_SET);

  if (length < 0) {
    return EXIT_FAILURE;
  }

  src = (char *)malloc(length + 1);

  size_t read_length = fread(src, 1, length, project_toml);

  if (read_length != (size_t)length) {
    return EXIT_FAILURE;
  }

  src[length] = '\0';

  token_steam = (Token *)malloc(sizeof(Token) * 100);

  int token_count = 0;
  Token tok = tokenize();
  while (tok.type != TokenType::TOK_EOF && token_count < 100) {
    token_steam[token_count++] = tok;
    tok = tokenize();
  }

  std::string binary_name = "";
  std::string compiler_name = "";

  for (int i = 0; i < token_count; i++) {
    if (strcmp(token_steam[i].lexeme, "name") == 0 && i + 2 < token_count) {
      binary_name = token_steam[i + 2].lexeme;
    }
    if (strcmp(token_steam[i].lexeme, "compiler") == 0 && i + 2 < token_count) {
      compiler_name = token_steam[i + 2].lexeme;
    }
  }

  std::string build_cmd = "mkdir -p .build/bin";
  std::string cmd =
      compiler_name + " -g -O0 src/main.cpp -o .build/bin/" + binary_name;

  printf("Executing: %s\n", build_cmd.c_str());
  int build_result = system(build_cmd.c_str());
  printf("Executing: %s\n", cmd.c_str());
  int result = system(cmd.c_str());

  if (build_result >= 0) {
    printf("Succesfully Created .build/bin\n");
  } else {
    printf("Failed to construct .build/bin\n");
    return build_result;
  }

  if (result >= 0) {
    printf("Succesfully build, %s\n", binary_name.c_str());
  } else {
    printf("Failed to build: %s\n", binary_name.c_str());
    return result;
  }

  return EXIT_SUCCESS;
}
