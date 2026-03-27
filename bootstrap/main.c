#include <stdio.h>
#include <stdlib.h>

int main(void) {

  printf("Bootstrapping forge...\n");

  const char *build_cmd = "mkdir -p .bootstrap/bin";
  const char *cmd = "g++ -std=c++20 src/main.cpp -o .bootstrap/bin/forge";

  int build_result = system(build_cmd);
  int result = system(cmd);

  if (build_result == 0) {
    printf("created build directory at .bootstrap/bin\n");
  } else {
    printf("build directory creation failed\n");
  }

  if (result == 0) {
    printf("success! stage 1 binary created at .bootstrap/bin/forge\n");
  } else {
    printf("Bootstrap failed\n");
  }

  return result;
}
